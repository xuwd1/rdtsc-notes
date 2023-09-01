#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <emmintrin.h>
#include <x86intrin.h>
#include <cpuid.h>

#include "core_routine.hpp"

/*
reference : This FANTASTIC tutorial made by intel
https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf
*/


constexpr size_t bound_of_loop = 1000;

double calc_var(uint64_t* vals, size_t n){
    double e_square_x = 0;
    double e_x_square = 0;
    for (size_t i=0; i<n; i++){
        e_square_x+=vals[i] * vals[i];
        e_x_square+=vals[i];
    }
    e_square_x = e_square_x/n;
    double t = e_x_square/n;
    e_x_square = t*t;
    return e_square_x - e_x_square;
}



int main(){
    uint64_t* deltas_arr = new uint64_t[bound_of_loop];

    #pragma unroll(32)
    for (size_t i =0 ; i<bound_of_loop; i++){

        deltas_arr[i] = measure_overhead();

    }
    
    
    for (size_t i=0; i<bound_of_loop;i++){
        std::printf("%lu ",deltas_arr[i]);
    }
    std::printf("\n");
    double var;
    var = calc_var(deltas_arr, bound_of_loop);
    std::printf("%f\n",var);
}