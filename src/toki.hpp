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
