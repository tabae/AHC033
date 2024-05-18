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
