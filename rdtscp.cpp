#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <emmintrin.h>
#include <x86intrin.h>
#include <cpuid.h>


/*
reference : This FANTASTIC tutorial made by intel
https://www.intel.com/content/dam/www/public/us/en/documents/white-papers/ia-32-ia-64-benchmark-code-execution-paper.pdf
*/

constexpr size_t size_of_stat = 100000;
constexpr size_t bound_of_loop = 1000;


inline volatile void clock(uint32_t& clk_hi, uint32_t& clk_lo){
    asm volatile(
        "rdtscp\n\t"
        "lfence"
        : "=d"(clk_hi), "=a"(clk_lo)
        :
        : "%rcx"
    );
}

int main(){
    uint32_t nonsense;
    uint64_t* detlas_arr = new uint64_t[bound_of_loop];
    volatile int32_t a =32;
    volatile int32_t b =32;
    volatile int32_t c =0;

    for (size_t i =0 ; i<bound_of_loop; i++){

        uint32_t cycles_high;
        uint32_t cycles_low;
        uint32_t cycles_high1, cycles_low1;
        cycles_high = 0;
        cycles_low = 0;

        clock(cycles_high,cycles_low);
        clock(cycles_high1,cycles_low1);

        uint64_t start = (uint64_t)cycles_high<<32 | cycles_low;
        uint64_t end  = (uint64_t)cycles_high1<<32 | cycles_low1;
        detlas_arr[i] = end-start;

    }
    //         asm volatile (
    // "RDTSCP\n\t"/*read the clock*/
    // "LFENCE\n\t"/*serialize*/
    // //"mov %%edx, %0\n\t"
    // //"mov %%eax, %1\n\t"
    // : "=d" (cycles_high), "=a"(cycles_low)
    // :
    // : "%rbx", "%rcx");



    // /*
    // Call the function to benchmark
    // */
    // #pragma unroll 
    // for (size_t i=0; i<16;i++){
    //     c+=a;

    // }
    
    

    // asm volatile (
    // "RDTSCP\n\t"/*read the clock*/
    // "LFENCE\n\t"
    // "mov %%edx, %0\n\t"
    // "mov %%eax, %1\n\t"
    // : "=r" (cycles_high1), "=r"
    // (cycles_low1):: "%rax", "%rbx", "%rcx", "%rdx");
    //     uint64_t start = (uint64_t)cycles_high<<32 | cycles_low;
    //     uint64_t end  = (uint64_t)cycles_high1<<32 | cycles_low1;
    //     detlas_arr[i] = end-start;
    // }

        //_mm_mfence();
        //uint64_t start = __rdtscp(&nonsense);
        //
        //_mm_mfence();
        //uint64_t end = __rdtscp(&nonsense);
        //
        //detlas_arr[i] = end-start;
    
    
    for (size_t i=0; i<bound_of_loop;i++){
        std::printf("%lu ",detlas_arr[i]);
    }
    std::printf("\n");
}