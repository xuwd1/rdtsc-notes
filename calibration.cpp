#include <ctime>
#include <cstdio>

int main(){

    std::timespec tt;
    clock_getres(CLOCK_MONOTONIC, &tt);
    printf("%ld\n",tt.tv_nsec);
    printf("%ld\n",tt.tv_sec);
    clock_gettime(CLOCK_MONOTONIC, &tt);
    printf("%ld\n",tt.tv_nsec);
    printf("%ld\n",tt.tv_sec);
    clock_gettime(CLOCK_MONOTONIC, &tt);
    printf("%ld\n",tt.tv_nsec);
    printf("%ld\n",tt.tv_sec);
    clock_gettime(CLOCK_MONOTONIC, &tt);
    printf("%ld\n",tt.tv_nsec);
    printf("%ld\n",tt.tv_sec);
    clock_gettime(CLOCK_MONOTONIC, &tt);
    printf("%ld\n",tt.tv_nsec);
    printf("%ld\n",tt.tv_sec);
}