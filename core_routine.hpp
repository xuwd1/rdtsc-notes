#pragma once

#include <cstdint>


inline volatile void clock(uint32_t& clk_hi, uint32_t& clk_lo){
    asm volatile(
        "rdtscp\n\t"
        "lfence\n\t"
        : "=d"(clk_hi), "=a"(clk_lo)
        :
        : "%rcx"
    );
}

inline volatile void clock_rdtsc(uint32_t& clk_hi, uint32_t& clk_lo){
    asm volatile(
        "lfence\n\t"
        "rdtsc\n\t"
        "lfence\n\t"
        : "=d"(clk_hi), "=a"(clk_lo)
        :
        :
    );
}

inline volatile uint64_t concat_clk(const uint32_t& clk_hi, const uint32_t& clk_lo){
    return (uint64_t)(clk_hi)<<32 | clk_lo;
} 

inline volatile uint64_t measure_overhead(){
    uint32_t ch1,cl1,ch2,cl2;
    clock(ch1,cl1);
    clock(ch2,cl2);
    uint64_t clk1 = concat_clk(ch1,cl1);
    uint64_t clk2 = concat_clk(ch2, cl2);
    uint64_t delta = clk2 - clk1; //clock is monotonic
    return delta;
}   