#include "common.hpp"
#include "solver_000.hpp"
#include "solver_001.hpp"
#include "toki.hpp"
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
