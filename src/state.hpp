#ifndef __STATE_HPP__
#define __STATE_HPP__

#include "common.hpp"
#include "ryuka.hpp"
#include <numeric>
#include <algorithm>

using namespace std;

extern Input in;
extern RandGenerator ryuka;

struct State {
    static constexpr long long inf = 1LL<<60;
    long long score;
    State() : score(-inf) {};
    long long calc_score();
    static State initState();
    static State generateState(const State& input_state);
};

long long State::calc_score() {
    return score;
}

State State::initState() {
    State res;
    res.calc_score();
    return res;
}

State State::generateState(const State& input_state) {
    State res = input_state;
    res.calc_score();
    return res;
}

#endif
