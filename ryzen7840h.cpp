#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <emmintrin.h>
#include <x86intrin.h>
#include <cpuid.h>

#include "core_routine.hpp"

constexpr size_t N=16384;

int main(){
    uint64_t* clocks = new uint64_t[N];
    uint32_t clk_hi,clk_lo;

    #pragma unroll(32)
    for(size_t i=0; i<N; i++){
    
        clock_rdtsc(clk_hi,clk_lo);
        clocks[i] = concat_clk(clk_hi, clk_lo);
    }
    
    uint64_t count=0;
    for (size_t i=0; i<N; i++){
        if (clocks[i]%38==0){
            count++;
        }
    }
    printf("Out of %lu samples %lu was multiple of 38\n",N,count);

}