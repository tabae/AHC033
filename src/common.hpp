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
    // move cranes
    for(int i = 0; i < n; i++) {
        // already disappear
        if(!cranes[i]->exist) {
            assert(actions[i] == ActionType::DESTROYED);
            continue;
        }
        // L,R,U,D
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
        // P
        if(actions[i] == ActionType::CATCH) {
            assert(container_pos[cranes[i]->i][cranes[i]->j]);
            assert(!cranes[i]->container);
            cranes[i]->container = move(container_pos[cranes[i]->i][cranes[i]->j]);
        }
        // Q
        if(actions[i] == ActionType::RELEASE) {
            assert(cranes[i]->container);
            assert(!container_pos[cranes[i]->i][cranes[i]->j]);
            container_pos[cranes[i]->i][cranes[i]->j] = move(cranes[i]->container);
            assert(container_pos[cranes[i]->i][cranes[i]->j]->i == cranes[i]->i &&
                   container_pos[cranes[i]->i][cranes[i]->j]->j == cranes[i]->j );
        }
        // B
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
    // watch status for debugging

    // collected containers
    cerr << "collected: " << collected_containers.size() << endl;
    for(int i = 0; i < n; i++) {
        cerr << " " << i << ": ";
        for(auto c: collected_containers[i]) cerr << c->id << ", ";
        cerr << endl;
    }
    cerr << endl;

    // queueing containers
    // TODO

    // container map
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

    // crane map
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