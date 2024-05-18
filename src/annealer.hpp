#ifndef __ANNEALER_HPP__
#define __ANNEALER_HPP__

#include "toki.hpp"
#include "ryuka.hpp"

extern Timer toki;
extern RandGenerator ryuka;

template<class STATE>
struct IterationControl {
    long long iteration_counter;
    long long swap_counter;
    double average_time;
    double start_time;
    IterationControl() : iteration_counter(0), swap_counter(0) {}
    STATE climb(double time_limit, STATE initial_state);
    STATE anneal(double time_limit, double temp_start, double temp_end, STATE initial_state);
};

template<class STATE>
STATE IterationControl<STATE>::climb(double time_limit, STATE initial_state) {
    start_time = toki.gettime();
    average_time = 0;
    STATE best_state = initial_state;
    double time_stamp = start_time;
    cerr << "Starts climbing...\n";
    while(time_stamp - start_time + average_time < time_limit) {
        STATE current_state = STATE::generateState(best_state);
        if(current_state.score > best_state.score) {
            swap(best_state, current_state);
            swap_counter++;
        }
        iteration_counter++;
        time_stamp = toki.gettime();
        average_time = (time_stamp - start_time) / iteration_counter;
    }
    cerr << "Iterated " << iteration_counter << " times and swapped " << swap_counter << " times.\n";
    return best_state;
}

template<class STATE>
STATE IterationControl<STATE>::anneal(double time_limit, double temp_start, double temp_end, STATE initial_state) {
    start_time = toki.gettime();
    average_time = 0;
    STATE best_state = initial_state;
    double elapsed_time = 0;
    cerr << "Starts annealing...\n";
    while(elapsed_time + average_time < time_limit) {
        double normalized_time = elapsed_time / time_limit;
        double temp_current = pow(temp_start, 1.0 - normalized_time) * pow(temp_end, normalized_time);
        STATE current_state = STATE::generateState(best_state);
        long long delta = current_state.score - best_state.score;
        if(delta > 0 || ryuka.pjudge(exp(1.0 * delta / temp_current)) ) {
            swap(best_state, current_state);
            swap_counter++;
        }
        iteration_counter++;
        elapsed_time = toki.gettime() - start_time;
        average_time = elapsed_time / iteration_counter;
    }
    cerr << "Iterated " << iteration_counter << " times and swapped " << swap_counter << " times.\n";
    return best_state;
}

#endif
