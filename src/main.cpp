#include "common.hpp"
#include "greedy.hpp"
#include <iostream>
using namespace std;

Input in;

int main() {

    in.read();

    vector<vector<ActionType>> ans = greedy_solve();
    
    common::print(ans);

}
