#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <queue>
#include <cassert>
#include <iomanip>

using namespace std;

constexpr int n = 5;
constexpr int MAX_TURN = 10000;

enum ObjectType {
    CRANE,
    CONTAINER,
};

enum CraneType {
    LARGE,
    SMALL,
};

enum CraneStatus {
    FREE,
    PRE_CATCH,
    CATCH_NOW,
    PRE_RELEASE,
    RELEASE_NOW,
};

enum ActionType {
    CATCH,
    RELEASE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    WAIT,
    BOMB,
    DESTROYED,
};

struct Object {
    int i, j;
    int id;
    ObjectType obj_type;
    Object(int i, int j, int id, ObjectType obj_type) : i(i), j(j), id(id), obj_type(obj_type) {}
    virtual ~Object() = default;
};

struct Container : Object {
    int out_i;
    Container(int i, int j, int id, int out_i) : Object(i, j, id, ObjectType::CONTAINER), out_i(out_i) {}
};

struct Crane : Object {
    CraneType crane_type;
    shared_ptr<Container> container;
    bool exist;
    CraneStatus status;
    int catch_i, catch_j;
    int release_i, release_j;
    int prev_container_id;
    Crane(int i, int j, int id, CraneType crane_type) : Object(i, j, id, ObjectType::CRANE), crane_type(crane_type), container(nullptr), exist(true), prev_container_id(-1) {} 
    void set_catch_and_release(int, int, int, int);
};

struct Input {
    vector<vector<int>> a;
    void read();
};

struct Terminal {
    int turn_count;
    vector<shared_ptr<Crane>> cranes;
    vector<shared_ptr<Container>> containers;
    vector<queue<shared_ptr<Container>>> container_queue;
    vector<vector<shared_ptr<Crane>>> crane_pos;
    vector<vector<shared_ptr<Container>>> container_pos;
    vector<vector<shared_ptr<Container>>> collected_containers;
    void init(const Input&);
    void update1();
    void update2(const vector<ActionType>&);
    void update3();
    void watch();
};

namespace common {
    char act2char(const ActionType);
    pair<int, int> act2move(const ActionType);
    int calc_out_i(int id);
    void print(const vector<vector<ActionType>>&);
};

void Input::read() {
    int n;
    cin >> n;
    a.resize(n, vector<int>(n));
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            cin >> a[i][j];
        }
    }
}

void Crane::set_catch_and_release(int _catch_i, int _catch_j, int _release_i, int _release_j) {
    assert(status == CraneStatus::FREE);
    status = CraneStatus::PRE_CATCH;
    catch_i = _catch_i;
    catch_j = _catch_j;
    release_i = _release_i;
    release_j = _release_j;
}

void common::print(const vector<vector<ActionType>>& actions) {
    for(int i = 0; i < n; i++) {
        for(const ActionType act: actions[i]) {
            if(act != ActionType::DESTROYED) {
                cout << common::act2char(act);
            }
        }
        cout << endl;
    }
}

void Terminal::init(const Input& in) {
    turn_count = 0;
    crane_pos.resize(n, vector<shared_ptr<Crane>>(n, nullptr));
    container_pos.resize(n, vector<shared_ptr<Container>>(n, nullptr));
    container_queue.resize(n);
    collected_containers.resize(n);
    cranes.resize(n);
    containers.resize(n*n);
    for(int i = 0; i < n; i++) {
        cranes[i] = make_shared<Crane>(i, 0, i, (i == 0 ? CraneType::LARGE : CraneType::SMALL));
    }
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            containers[in.a[i][j]] = make_shared<Container>(i, 0, in.a[i][j], common::calc_out_i(in.a[i][j]));
            container_queue[i].push(containers[in.a[i][j]]);
        }
    }
}

void Terminal::update1() {
    turn_count++;
    for(int i = 0; i < n; i++) {
        if(container_queue[i].empty()) {
            continue;
        }
        shared_ptr<Container> c = container_queue[i].front();
        if( (!container_pos[c->i][c->j]) && 
            ((!crane_pos[c->i][c->j]) || (!crane_pos[c->i][c->j]->container)) ) {
            container_pos[c->i][c->j] = move(c);
            container_queue[i].pop();
        }   
    }
}

void Terminal::update2(const vector<ActionType>& actions) {
    vector next_crane_pos(n, vector<shared_ptr<Crane>>(n, nullptr));
    for(int i = 0; i < n; i++) {
        if(!cranes[i]->exist) {
            assert(actions[i] == ActionType::DESTROYED);
            continue;
        }
        auto [di, dj] = common::act2move(actions[i]);
        const int next_i = cranes[i]->i + di;
        const int next_j = cranes[i]->j + dj;
        next_crane_pos[next_i][next_j] = cranes[i];
        cranes[i]->i = next_i;
        cranes[i]->j = next_j;
        if(cranes[i]->container) {
            cranes[i]->container->i = next_i;
            cranes[i]->container->j = next_j;
        }
        assert(cranes[i] == next_crane_pos[next_i][next_j]);
        if(cranes[i]->status == CraneStatus::PRE_CATCH) {
            if(cranes[i]->i == cranes[i]->catch_i && cranes[i]->j == cranes[i]->catch_j) {
                cranes[i]->status = CraneStatus::CATCH_NOW;
            }
        }
        else if(cranes[i]->status == CraneStatus::PRE_RELEASE) {
            if(cranes[i]->i == cranes[i]->release_i && cranes[i]->j == cranes[i]->release_j) {
                cranes[i]->status = CraneStatus::RELEASE_NOW;
            }
        }
        if(actions[i] == ActionType::CATCH) {
            assert(container_pos[cranes[i]->i][cranes[i]->j]);
            assert(!cranes[i]->container);
            cranes[i]->container = move(container_pos[cranes[i]->i][cranes[i]->j]);
            cranes[i]->prev_container_id = cranes[i]->container->id;
            cranes[i]->status = CraneStatus::PRE_RELEASE;
        }
        if(actions[i] == ActionType::RELEASE) {
            assert(cranes[i]->container);
            if(container_pos[cranes[i]->i][cranes[i]->j]) {
                cerr << cranes[i]->status << endl;
                cerr << cranes[i]->container->id << ", " << container_pos[cranes[i]->i][cranes[i]->j]->id << endl;
            }
            assert(!container_pos[cranes[i]->i][cranes[i]->j]);
            container_pos[cranes[i]->i][cranes[i]->j] = move(cranes[i]->container);
            assert(container_pos[cranes[i]->i][cranes[i]->j]->i == cranes[i]->i &&
                   container_pos[cranes[i]->i][cranes[i]->j]->j == cranes[i]->j );
            cranes[i]->status = CraneStatus::FREE;
        }
        if(actions[i] == ActionType::BOMB) {
            assert(!cranes[i]->container);
            cranes[i]->exist = false;
            next_crane_pos[cranes[i]->i][cranes[i]->j] = nullptr;
        }
    }
    crane_pos = move(next_crane_pos);
}

void Terminal::update3() {
    for(int i = 0; i < n; i++) {
        if(container_pos[i][n-1]) {
            collected_containers[i].push_back(container_pos[i][n-1]);
            container_pos[i][n-1] = nullptr;
        }
    }
}

void Terminal::watch() {

    cerr << "collected: " << collected_containers.size() << endl;
    for(int i = 0; i < n; i++) {
        cerr << " " << i << ": ";
        for(auto c: collected_containers[i]) cerr << c->id << ", ";
        cerr << endl;
    }
    cerr << endl;


    cerr << "conatiner map: " << endl;
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            if(container_pos[i][j]) {
                cerr << setw(2) << container_pos[i][j]->id;
            } else {
                cerr << "  ";
            }
            cerr << "|"; 
        }
        cerr << endl;
    }
    cerr << endl;

    cerr << "crane map: " << endl;
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            if(crane_pos[i][j]) {
                cerr << setw(2) << crane_pos[i][j]->id;
            } else {
                cerr << "  ";
            }
            cerr << "|"; 
        }
        cerr << endl;
    }
    cerr << endl;
}

char common::act2char(const ActionType act) {
    if(act == ActionType::CATCH) return 'P';
    if(act == ActionType::RELEASE) return 'Q';
    if(act == ActionType::UP) return 'U';
    if(act == ActionType::DOWN) return 'D';
    if(act == ActionType::LEFT) return 'L';
    if(act == ActionType::RIGHT) return 'R';
    if(act == ActionType::WAIT) return '.';
    if(act == ActionType::BOMB) return 'B';
    return '?';
}

pair<int,int> common::act2move(const ActionType act) {
    int i = 0;
    int j = 0;
    if(act == ActionType::DOWN) i = 1;
    if(act == ActionType::UP) i = -1;
    if(act == ActionType::LEFT) j = -1;
    if(act == ActionType::RIGHT) j = 1;
    return make_pair(i, j);
}

int common::calc_out_i(int id) {
    return id / n;
}

#endif
#ifndef __SOLVER_000_HPP__
#define __SOLVER_000_HPP__

#include <numeric>
#include <algorithm>
#include <set>

extern Input in;


namespace solver_000 {

void copy2res(vector<vector<ActionType>>& res, const vector<ActionType>& actions) {
    for(int i = 0; i < n; i++) {
        res[i].push_back(actions[i]);
    }
}

vector<vector<ActionType>> solve() {

    vector<vector<ActionType>> res(n);
    
    Terminal term;
    term.init(in);    


    for (int j = 3; j >= 1; j--) {
        const int m1 = j;
        const int m2 = m1 + 1;
        const int m3 = m2 + j;
        for(int k = 0; k <= m3; k++) {
            term.update1();
            
            ActionType act = ActionType::LEFT;
            if(k == 0) {
                act = ActionType::CATCH;
                for(int i = 0; i < n; i++) {
                    assert(term.cranes[i]->i == i);
                    assert(term.cranes[i]->j == 0);
                }
            } else if(1 <= k && k <= m1) {
                act = ActionType::RIGHT;
            } else if(k == m2) {
                act = ActionType::RELEASE;
                for(int i = 0; i < n; i++) {
                    assert(term.cranes[i]->i == i);
                    assert(term.cranes[i]->j == j);
                }
            } else if(m2+1 <= k && k <= m3) {
                act = ActionType::LEFT;
            }

            vector<ActionType> actions(n, act);
            copy2res(res, actions);
            term.update2(actions);            
            term.update3();
        }
    }

    {
        term.update1();
        vector<ActionType> actions(n, ActionType::WAIT);
        for(int i = 1; i < n; i++) {
            actions[i] = ActionType::BOMB;
        }
        copy2res(res, actions);
        term.update2(actions);
        term.update3();
    }

    while(term.turn_count < MAX_TURN) {

        vector<int> next_c(n, 100);
        set<int> remains;
        for(int i = 0; i < n*n; i++) remains.insert(i);
        for(int i = 0; i < n; i++) {
            for(auto p: term.collected_containers[i]) {
                remains.erase(p->id);
            }
        }
        if(remains.empty()) {
            cerr << "all containers gone" << endl;
            break;
        }
        for(int id: remains) {
            int i = common::calc_out_i(id);
            next_c[i] = min(next_c[i], id);
        }
        
        int c = -1;
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < n-1; j++) {
                for(int k = 0; k < n; k++) {
                    if(term.container_pos[i][j] && term.container_pos[i][j]->id == next_c[k]) {
                        c = next_c[k];
                        break;
                    }
                }
                if(c != -1) break;
            }
            if(c != -1) break;
        }

        if(c == -1) {
            cerr << "cannot found target container (c=-1)" << endl;
            break;
        }

        cerr << "c = " << c << endl;

        auto catch_and_release = [&](int c, int goal_i, int goal_j) -> void {

            {
                const int next_i = term.containers[c]->i;
                const int next_j = term.containers[c]->j;
                while(next_i != term.cranes[0]->i) {
                    vector<ActionType> actions(n, ActionType::DESTROYED);
                    const ActionType act = (next_i > term.cranes[0]->i ? ActionType::DOWN : ActionType::UP);
                    actions[0] = act;
                    term.update1();
                    term.update2(actions);
                    copy2res(res, actions);
                    term.update3();
                }
                while(next_j != term.cranes[0]->j) {
                    vector<ActionType> actions(n, ActionType::DESTROYED);
                    const ActionType act = (next_j > term.cranes[0]->j ? ActionType::RIGHT : ActionType::LEFT);
                    actions[0] = act;
                    term.update1();
                    term.update2(actions);
                    copy2res(res, actions);
                    term.update3();
                }
            }

            {
                vector<ActionType> actions(n, ActionType::DESTROYED);
                actions[0] = ActionType::CATCH;
                term.update1();
                term.update2(actions);
                copy2res(res, actions);
                term.update3();
            }

            {
                const int next_i = goal_i;
                const int next_j = goal_j;
                while(next_i != term.cranes[0]->i) {
                    vector<ActionType> actions(n, ActionType::DESTROYED);
                    const ActionType act = (next_i > term.cranes[0]->i ? ActionType::DOWN : ActionType::UP);
                    actions[0] = act;
                    term.update1();
                    term.update2(actions);
                    copy2res(res, actions);
                    term.update3();
                }
                while(next_j != term.cranes[0]->j) {
                    vector<ActionType> actions(n, ActionType::DESTROYED);
                    const ActionType act = (next_j > term.cranes[0]->j ? ActionType::RIGHT : ActionType::LEFT);
                    actions[0] = act;
                    term.update1();
                    term.update2(actions);
                    copy2res(res, actions);
                    term.update3();
                }
            }

            {
                vector<ActionType> actions(n, ActionType::DESTROYED);
                actions[0] = ActionType::RELEASE;
                term.update1();
                term.update2(actions);
                copy2res(res, actions);
                term.update3();
            }
        };

        const int catch_i = term.containers[c]->i;
        const int catch_j = term.containers[c]->j;
        catch_and_release(c, term.containers[c]->out_i, n-1);

        for(int i = 0; i < n; i++) {
            if(term.container_pos[i][0] && !term.container_queue[i].empty()
                && !term.container_pos[catch_i][catch_j]) {
                catch_and_release(term.container_pos[i][0]->id, catch_i, catch_j);
            }
        }
        
        cerr << "turn_count = " << term.turn_count << endl;
    }
    return res;

}

} // namespace solver_000

#endif
#ifndef __SOLVER_001_HPP__
#define __SOLVER_001_HPP__

#ifndef __RYUKA_HPP__
#define __RYUKA_HPP__

#include <random>
using namespace std;

struct RandGenerator {

    random_device seed_gen;
    mt19937 engine;
    mt19937_64 engine64;
    static const int pshift = 1000000000;
    
    RandGenerator() : engine(seed_gen()), engine64(seed_gen()) {}
    
    int rand(int mod) {
        return engine() % mod;
    }
    
    long long randll(long long mod) {
        return engine64() % mod;
    } 
    
    bool pjudge(double p) {
        int p_int;
        if(p > 1) p_int = pshift;
        else p_int = p * pshift;
        return rand(pshift) < p_int;
    }

} ryuka;

#endif
#include <numeric>
#include <algorithm>
#include <set>

extern Input in;
extern RandGenerator ryuka;

namespace sovler_001 {

struct CR_task {
    int catch_i, catch_j;
    int release_i, release_j;
    bool large_job;
    CR_task(int catch_i, int catch_j, int release_i, int release_j, bool large_job) : 
        catch_i(catch_i), catch_j(catch_j), release_i(release_i), release_j(release_j), large_job(large_job) {}
};

void copy2res(vector<vector<ActionType>>& res, const vector<ActionType>& actions) {
    for(int i = 0; i < n; i++) {
        res[i].push_back(actions[i]);
    }
}

ActionType get_next_action(int i, const vector<vector<shared_ptr<Crane>>>& next_crane_pos, const Terminal& term, const vector<vector<ActionType>>& res) {
    shared_ptr<Crane> crane = term.cranes[i];
    if(!crane->exist) {
        return ActionType::DESTROYED;
    }
    auto random_walk = [&]() -> ActionType {
        constexpr int di[4] = {1, -1,  0, 0};
        constexpr int dj[4] = {0,  0, -1, 1};
        vector<int> k_idx = {0, 1, 2, 3};
        shuffle(k_idx.begin(), k_idx.end(), ryuka.engine);
        constexpr ActionType act_types[4] = {ActionType::DOWN, ActionType::UP, ActionType::LEFT, ActionType::RIGHT};
        for(int k : k_idx) {
            const int next_i = crane->i + di[k];
            const int next_j = crane->j + dj[k];
            if(next_i < 0 || next_j < 0 || next_i >= n || next_j >= n) continue;
            if(!term.crane_pos[next_i][next_j] && !next_crane_pos[next_i][next_j]) {
                return act_types[k];
            }
        }
        return ActionType::BOMB;
    };       
    if(crane->status == CraneStatus::FREE) {
        assert(!crane->container);
        /*
        if(!next_crane_pos[crane->i][crane->j]) {
            return ActionType::WAIT;
        }
        */
        int container_count = 0;
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < n; j++) {
                if(term.container_pos[i][j]) {
                    container_count++;
                }
            }
        }
        int crane_count = 0;
        for(int i = 0; i < n; i++) {
            if(term.cranes[i]->exist) {
                crane_count++;
            }
        }
        if(container_count < crane_count && i != 0) {
            return ActionType::BOMB;
        }
        return random_walk();
    }
    else if(crane->status == CraneStatus::PRE_CATCH) {
        const int di = crane->catch_i - crane->i;
        const int dj = crane->catch_j - crane->j;
        const int ord = ryuka.rand(2);
        for(int k = 0; k < 2; k++) {
            if(di != 0 && k == ord) {
                const int next_i = crane->i + (di > 0 ? 1 : -1);
                const int next_j = crane->j;
                if(!term.crane_pos[next_i][next_j] && !next_crane_pos[next_i][next_j]) {
                    return (di > 0 ? ActionType::DOWN : ActionType::UP);
                }
            }
            if(dj != 0 && k != ord) {
                const int next_i = crane->i;
                const int next_j = crane->j + (dj > 0 ? 1 : -1);
                if(!term.crane_pos[next_i][next_j] && !next_crane_pos[next_i][next_j]) {
                    return (dj > 0 ? ActionType::RIGHT : ActionType::LEFT);
                }
            }
        }
        if(di == 0 && dj == 0) {
            return ActionType::CATCH;
        }
        if(crane->crane_type == CraneType::SMALL) {
            return random_walk();
        }
        return ActionType::WAIT;
    } 
    else if(crane->status == CraneStatus::CATCH_NOW) {
        assert(crane->i == crane->catch_i && crane->j == crane->catch_j);
        if(!term.container_pos[crane->i][crane->j]) {
            cerr << crane->i << ", " << crane->j << endl;
            common::print(res);
        }
        assert(term.container_pos[crane->i][crane->j]);
        assert(!crane->container);
        return ActionType::CATCH;
    }
    else if(crane->status == CraneStatus::PRE_RELEASE) {
        const int di = crane->release_i - crane->i;
        const int dj = crane->release_j - crane->j;
        const int ord = ryuka.rand(2);
        for(int k = 0; k < 2; k++) {
            if(di != 0 && k == ord) {
                const int next_i = crane->i + (di > 0 ? 1 : -1);
                const int next_j = crane->j;
                if(!term.crane_pos[next_i][next_j] && !next_crane_pos[next_i][next_j]) {
                    return (di > 0 ? ActionType::DOWN : ActionType::UP);
                }
            }
            if(dj != 0 && k != ord) {
                const int next_i = crane->i;
                const int next_j = crane->j + (dj > 0 ? 1 : -1);
                if(!term.crane_pos[next_i][next_j] && !next_crane_pos[next_i][next_j]) {
                    return (dj > 0 ? ActionType::RIGHT : ActionType::LEFT);
                }
            }
        }
        if(di == 0 && dj == 0) {
            return ActionType::RELEASE;
        }
        if(crane->crane_type == CraneType::SMALL) {
            if(crane->container) {
                return ActionType::RELEASE;
            }
            return random_walk();
        }
        return ActionType::WAIT;
    }
    else if(crane->status == CraneStatus::RELEASE_NOW) {
        assert(crane->i == crane->release_i && crane->j == crane->release_j);
        assert(crane->container);
        return ActionType::RELEASE;
    }

    cerr << "get_next_action: failed to find next action" << endl;
    return ActionType::WAIT;
}


vector<vector<ActionType>> solve() {

    vector<vector<ActionType>> res(n);
    
    Terminal term;
    term.init(in);    


    for (int j = 3; j >= 1; j--) {
        const int m1 = j;
        const int m2 = m1 + 1;
        const int m3 = m2 + j;
        for(int k = 0; k <= m3; k++) {
            term.update1();
            
            ActionType act = ActionType::LEFT;
            if(k == 0) {
                act = ActionType::CATCH;
                for(int i = 0; i < n; i++) {
                    assert(term.cranes[i]->i == i);
                    assert(term.cranes[i]->j == 0);
                }
            } else if(1 <= k && k <= m1) {
                act = ActionType::RIGHT;
            } else if(k == m2) {
                act = ActionType::RELEASE;
                for(int i = 0; i < n; i++) {
                    assert(term.cranes[i]->i == i);
                    assert(term.cranes[i]->j == j);
                }
            } else if(m2+1 <= k && k <= m3) {
                act = ActionType::LEFT;
            }

            vector<ActionType> actions(n, act);
            copy2res(res, actions);
            term.update2(actions);            
            term.update3();
        }
    }

    /*
    {
        term.update1();
        vector<ActionType> actions(n, ActionType::WAIT);
        for(int i = 1; i < n; i++) {
            actions[i] = ActionType::BOMB;
        }
        copy2res(res, actions);
        term.update2(actions);
        term.update3();
    }
    */

    vector<CR_task> cr_tasks;
    auto erase_cr_tasks = [&](int catch_i, int catch_j) -> void {
        vector<CR_task> new_cr_tasks;
        for(const CR_task cr: cr_tasks) {
            if(cr.catch_i == catch_i && cr.catch_j == catch_j) {
                ;
            } else {
                new_cr_tasks.push_back(cr);
            }
        }
        swap(new_cr_tasks, cr_tasks);
    };

    auto check_conflict = [&](int i, int j, bool skip_large_job = false) -> bool {
        for(const CR_task cr : cr_tasks) {
            if(skip_large_job && cr.large_job) {
                continue;
            }
            if(cr.catch_i == i && cr.catch_j == j) {
                return false;
            }
            if(cr.release_i == i && cr.release_j == j) {
                return false;
            }
        }
        for(const shared_ptr<Crane> c : term.cranes) {
            if(c->status != CraneStatus::FREE && c->exist) {
                if( c->status == CraneStatus::PRE_CATCH || c->status == CraneStatus::CATCH_NOW) {
                    if(c->catch_i == i && c->catch_j == j) {
                        return false;
                    }
                }
                if(c->release_i == i && c->release_j == j) {
                    return false;
                }
            }
        }
        return true;
    };

    while(term.turn_count < MAX_TURN) {

        term.watch();

        vector<int> next_c(n, 100);
        set<int> remains;
        for(int i = 0; i < n*n; i++) remains.insert(i);
        for(int i = 0; i < n; i++) {
            for(auto p: term.collected_containers[i]) {
                remains.erase(p->id);
            }
        }
        if(remains.empty()) {
            cerr << "all containers gone" << endl;
            break;
        }
        for(int id: remains) {
            int i = common::calc_out_i(id);
            next_c[i] = min(next_c[i], id);
        }
        
        auto update_cr_tasks = [&]() -> void {
            for(int i = 0; i < n; i++) {
                for(int j = 0; j < n-1; j++) {
                    for(int r = 0; r < n; r++) {
                        if(check_conflict(i, j) && term.container_pos[i][j] && term.container_pos[i][j]->id == next_c[r]) {
                            cr_tasks.push_back(CR_task(i, j, term.container_pos[i][j]->out_i, n-1, true));
                            break;
                        }   
                    }
                }
            }
            for(int i = 0; i < n; i++) {
                for(int j = 0; j < n-1; j++) {
                    if(check_conflict(i, j) && !term.container_pos[i][j] && !term.crane_pos[i][j]) {
                        constexpr int di[2] = {1, -1};
                        constexpr int dj[2] = {0,  0};
                        for(int k = 0; k < 2; k++) {
                            const int adj_i = i + di[k];
                            const int adj_j = j + dj[k];
                            auto is_closer_than_cur = [&](int out_i, int cur_i, int next_i) -> bool {
                                const int diff_cur = abs(out_i - cur_i);
                                const int diff_next = abs(out_i - next_i);
                                return diff_next < diff_cur;
                            };
                            if( adj_i >= 0 && adj_j >= 0 && adj_i < n && adj_j < n &&
                                term.container_pos[adj_i][adj_j] &&
                                check_conflict(adj_i, adj_j, true) &&
                                (is_closer_than_cur(term.container_pos[adj_i][adj_j]->out_i, adj_i, i) || !term.container_queue[i].empty())) 
                            {
                                cr_tasks.push_back(CR_task(adj_i, adj_j, i, j, false));
                                break;
                            }
                        }
                    }
                }
            }
            for(int i = 0; i < n; i++) {
                for(int j = 1; j < n-1; j++) {
                    if(check_conflict(i, j) && !term.crane_pos[i][j] && !term.container_pos[i][j]) {
                        const int adj_i = i;
                        const int adj_j = j - 1;
                        if( adj_i >= 0 && adj_j >= 0 && adj_i < n && adj_j < n &&
                            term.container_pos[adj_i][adj_j] &&
                            check_conflict(adj_i, adj_j, true))
                        {
                            cr_tasks.push_back(CR_task(adj_i, adj_j, i, j, false));
                            break;
                        }
                    }
                }
            }
        };

        term.update1();
        update_cr_tasks();
        cerr << "cr_tasks.size() = " << cr_tasks.size() << endl;
        for(auto cr: cr_tasks) {
            cerr << cr.catch_i << "," << cr.catch_j << "->" << cr.release_i << "," << cr.release_j << endl;
        }
        for(int i = 0; i < n; i++) {
            cerr << "crane #" << i << ": " << term.cranes[i]->status << endl;
        }

        vector<ActionType> actions(n, ActionType::WAIT);
        vector<vector<shared_ptr<Crane>>> next_crane_pos(n, vector<shared_ptr<Crane>>(n, nullptr)); 

        for(int i = 0; i < n; i++) {
            if(!term.cranes[i]->exist) {
                actions[i] = ActionType::DESTROYED;
                continue;
            }
            if(term.cranes[i]->status == CraneStatus::FREE) {
                int best_task_id = -1;
                int min_dist = 100000;
                for(int t = 0; t < cr_tasks.size(); t++) {
                    if(!term.container_pos[cr_tasks[t].catch_i][cr_tasks[t].catch_j]) {
                        continue;
                    }
                    if(i != 0 && cr_tasks[t].large_job) {
                        continue;
                    }
                    int dist =  abs(term.cranes[i]->i - cr_tasks[t].catch_i) +
                                abs(term.cranes[i]->j - cr_tasks[t].catch_j);
                    if(term.cranes[i]->prev_container_id == term.container_pos[cr_tasks[t].catch_i][cr_tasks[t].catch_j]->id) {
                        if(term.cranes[i]->crane_type == CraneType::LARGE) {
                            dist += 1000;
                        } else {
                            dist += 1000000;
                        }
                    }
                    if(min_dist > dist) {
                        min_dist = dist;
                        best_task_id = t;
                    }
                }
                if(best_task_id != -1) {
                    const CR_task cr = cr_tasks[best_task_id];
                    cerr << "cr " << i << ":" << cr.catch_i << "," << cr.catch_j << "->" << cr.release_i << "," << cr.release_j << endl;
                    term.cranes[i]->set_catch_and_release(cr.catch_i, cr.catch_j, cr.release_i, cr.release_j);
                    erase_cr_tasks(cr.catch_i, cr.catch_j);
                }
            }
            ActionType act = get_next_action(i, next_crane_pos, term, res);
            actions[i] = act;

            {
                auto [di, dj] = common::act2move(actions[i]);
                const int next_i = term.cranes[i]->i + di;
                const int next_j = term.cranes[i]->j + dj;
                next_crane_pos[next_i][next_j] = term.cranes[i]; 
            }
        } 

        term.update2(actions);
        term.update3();
        copy2res(res, actions);

        cerr << "turn_count (solver_001) = " << term.turn_count << endl;
    }
    return res;

}

}; // namespace solver_001

#endif
#ifndef __TOKI_HPP__
#define __TOKI_HPP__

#include <sys/time.h>
#include <cstddef>

struct Timer {

    double global_start;
    
    double gettime() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec * 1e-6;
    }
    
    void init() {
        global_start = gettime();
    }
    
    double elapsed() {
        return gettime() - global_start;
    }
} toki;

#endif
#include <iostream>
using namespace std;

Input in;
extern Timer toki;

int main() {

    toki.init();
    in.read();

    vector<vector<ActionType>> ans = solver_000::solve();
    vector<vector<ActionType>> ans_001 = sovler_001::solve();
    if(ans_001.front().size() < ans.front().size()) {
        ans = ans_001;
    }
    
    common::print(ans);

}