#ifndef __GREEDY_HPP__
#define __GREEDY_HPP__

#include "common.hpp"
#include "ryuka.hpp"
#include <numeric>
#include <algorithm>
#include <set>

extern Input in;
extern RandGenerator ryuka;

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
    // すでに爆破済みの場合
    if(!crane->exist) {
        return ActionType::DESTROYED;
    }
    // その場でキープしていることが邪魔になる場合、四方向で移動できる方向に移動する
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
        // 一つも移動先がない場合、あきらめて爆破する
        return ActionType::BOMB;
    };       
    // することがない場合
    if(crane->status == CraneStatus::FREE) {
        assert(!crane->container);
        // することがなければ、その場をキープ
        /*
        if(!next_crane_pos[crane->i][crane->j]) {
            return ActionType::WAIT;
        }
        */
        // 残りのコンテナの数よりクレーンが大きければ爆破
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
    // CATCHに向かっている場合
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
        // 大クレーンの進路を妨げている場合、リリースさせる
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
        // 大クレーンの進路を妨げている場合、リリースさせる
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


vector<vector<ActionType>> greedy_solve() {

    vector<vector<ActionType>> res(n);
    
    Terminal term;
    term.init(in);    

    // 1. すべてのクレーンを使って、4つを倉庫内に並べる（3+2+1ターン）
    // (廃止) 2. 小クレーンはこの時点で爆破する
    // 3. 大クレーンを使って、搬出街のものを搬出口へもっていく
    // 4. 左が空いているものについて、大クレーンを使って、横にずらず。
    // 5. 3. に戻る
    // 6. すべてがなくなったら終了

    // step 1
    for (int j = 3; j >= 1; j--) {
        const int m1 = j;
        const int m2 = m1 + 1;
        const int m3 = m2 + j;
        // k = 0: コンテナをつかむ
        // k = 1 ~ m1 左に移動
        // k = m2: コンテナを話す
        // k = m2+1 ~ m3: 右に移動
        // k = m3+1, コンテナをつかむ…
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

    // step2
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

    // iterates step3 and step4
    while(term.turn_count < MAX_TURN) {

        term.watch();

        // 待機列
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
            // 搬出待ちのコンテナを移動させる
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
            // 空いているマスに上下で移動できるものがあれば移動させる
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
            // 左から右へ移動させることができる場合、移動させる。
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

        // 移動タスクのリストを更新する
        term.update1();
        update_cr_tasks();
        cerr << "cr_tasks.size() = " << cr_tasks.size() << endl;
        for(auto cr: cr_tasks) {
            cerr << cr.catch_i << "," << cr.catch_j << "->" << cr.release_i << "," << cr.release_j << endl;
        }
        for(int i = 0; i < n; i++) {
            cerr << "crane #" << i << ": " << term.cranes[i]->status << endl;
        }

        // 各クレーンの行動を決定する。
        vector<ActionType> actions(n, ActionType::WAIT);
        vector<vector<shared_ptr<Crane>>> next_crane_pos(n, vector<shared_ptr<Crane>>(n, nullptr)); 

        for(int i = 0; i < n; i++) {
            if(!term.cranes[i]->exist) {
                actions[i] = ActionType::DESTROYED;
                continue;
            }
            // 手持無沙汰な場合、適当なタスクをあてがう。
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
                    //cr_tasks.erase(cr_tasks.begin() + best_task_id);
                    erase_cr_tasks(cr.catch_i, cr.catch_j);
                }
            }
            ActionType act = get_next_action(i, next_crane_pos, term, res);
            actions[i] = act;

            // next_posを更新する
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

/*
        // まず、移動させるコンテナを決定する
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

        // 順番以外の場合はとりあえず、終了にする
        if(c == -1) {
            cerr << "cannot found target container (c=-1)" << endl;
            break;
        }

        cerr << "c = " << c << endl;

        auto catch_and_release = [&](int c, int goal_i, int goal_j) -> void {

            // 移動させるコンテナの場所まで移動する。
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

            // つかむ
            {
                vector<ActionType> actions(n, ActionType::DESTROYED);
                actions[0] = ActionType::CATCH;
                term.update1();
                term.update2(actions);
                copy2res(res, actions);
                term.update3();
            }

            // 搬出口まで移動する。
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

            // はなす
            {
                vector<ActionType> actions(n, ActionType::DESTROYED);
                actions[0] = ActionType::RELEASE;
                term.update1();
                term.update2(actions);
                copy2res(res, actions);
                term.update3();
            }
        };

        // step 3
        const int catch_i = term.containers[c]->i;
        const int catch_j = term.containers[c]->j;
        catch_and_release(c, term.containers[c]->out_i, n-1);

        // step 4
        for(int i = 0; i < n; i++) {
            if(term.container_pos[i][0] && !term.container_queue[i].empty()
                && !term.container_pos[catch_i][catch_j]) {
                catch_and_release(term.container_pos[i][0]->id, catch_i, catch_j);
            }
        }      
*/

        cerr << "turn_count = " << term.turn_count << endl;
    }
    return res;

}

#endif