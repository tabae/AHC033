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
    Crane(int i, int j, int id, CraneType crane_type) : Object(i, j, id, ObjectType::CRANE), crane_type(crane_type), container(nullptr), exist(true) {} 
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
        if(actions[i] == ActionType::CATCH) {
            assert(container_pos[cranes[i]->i][cranes[i]->j]);
            assert(!cranes[i]->container);
            cranes[i]->container = move(container_pos[cranes[i]->i][cranes[i]->j]);
        }
        if(actions[i] == ActionType::RELEASE) {
            assert(cranes[i]->container);
            assert(!container_pos[cranes[i]->i][cranes[i]->j]);
            container_pos[cranes[i]->i][cranes[i]->j] = move(cranes[i]->container);
            assert(container_pos[cranes[i]->i][cranes[i]->j]->i == cranes[i]->i &&
                   container_pos[cranes[i]->i][cranes[i]->j]->j == cranes[i]->j );
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
#ifndef __GREEDY_HPP__
#define __GREEDY_HPP__

#include <numeric>
#include <algorithm>
#include <set>

extern Input in;

void copy2rec(vector<vector<ActionType>>& res, const vector<ActionType>& actions) {
    for(int i = 0; i < n; i++) {
        res[i].push_back(actions[i]);
    }
}

vector<vector<ActionType>> greedy_solve() {

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
            copy2rec(res, actions);
            term.update2(actions);            
            term.update3();
        }
    }
    term.watch();

    {
        term.update1();
        vector<ActionType> actions(n, ActionType::WAIT);
        for(int i = 1; i < n; i++) {
            actions[i] = ActionType::BOMB;
        }
        copy2rec(res, actions);
        term.update2(actions);
        term.update3();
    }
    term.watch();

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
                    copy2rec(res, actions);
                    term.update3();
                }
                while(next_j != term.cranes[0]->j) {
                    vector<ActionType> actions(n, ActionType::DESTROYED);
                    const ActionType act = (next_j > term.cranes[0]->j ? ActionType::RIGHT : ActionType::LEFT);
                    actions[0] = act;
                    term.update1();
                    term.update2(actions);
                    copy2rec(res, actions);
                    term.update3();
                }
            }

            {
                vector<ActionType> actions(n, ActionType::DESTROYED);
                actions[0] = ActionType::CATCH;
                term.watch();
                term.update1();
                term.watch();
                term.update2(actions);
                term.watch();
                copy2rec(res, actions);
                term.update3();
                term.watch();
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
                    copy2rec(res, actions);
                    term.update3();
                }
                while(next_j != term.cranes[0]->j) {
                    vector<ActionType> actions(n, ActionType::DESTROYED);
                    const ActionType act = (next_j > term.cranes[0]->j ? ActionType::RIGHT : ActionType::LEFT);
                    actions[0] = act;
                    term.update1();
                    term.update2(actions);
                    copy2rec(res, actions);
                    term.update3();
                }
            }

            {
                vector<ActionType> actions(n, ActionType::DESTROYED);
                actions[0] = ActionType::RELEASE;
                term.update1();
                term.update2(actions);
                copy2rec(res, actions);
                term.update3();
            }
            term.watch();
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

#endif
#include <iostream>
using namespace std;

Input in;

int main() {

    in.read();

    vector<vector<ActionType>> ans = greedy_solve();
    
    common::print(ans);

}