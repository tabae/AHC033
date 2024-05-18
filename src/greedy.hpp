#ifndef __GREEDY_HPP__
#define __GREEDY_HPP__

#include "common.hpp"
#include <numeric>
#include <algorithm>
#include <set>

extern Input in;

void copy2res(vector<vector<ActionType>>& res, const vector<ActionType>& actions) {
    for(int i = 0; i < n; i++) {
        res[i].push_back(actions[i]);
    }
}

vector<vector<ActionType>> greedy_solve() {

    vector<vector<ActionType>> res(n);
    
    Terminal term;
    term.init(in);    

    // 1. すべてのクレーンを使って、4つを倉庫内に並べる（3+2+1ターン）
    // 2. 小クレーンはこの時点で爆破する
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

    // iterates step3 and step4
    while(term.turn_count < MAX_TURN) {

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
        
        cerr << "turn_count = " << term.turn_count << endl;
    }
    return res;

}

#endif