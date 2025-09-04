#include "utils.h"
#include <stdio.h>
#include <string.h>

char a[BENCHMARK_N];

int main(int argc, char *argv[]) {
    // Fill buffer with nonzero chars and null-terminate
    for (int i = 0; i < BENCHMARK_N - 1; ++i)
        a[i] = 'a';
    a[BENCHMARK_N - 1] = '\0';

    unsigned long long st1 = get_cpu_time();
    for (int i = 0; i < BENCHMARK_MICRO_REPEATS; ++i) {
        volatile size_t len = strlen(a);
        DoNotOptimize(len);
    }
    unsigned long long et1 = get_cpu_time();

    printf("misc.strlen:\t%d %d %llu\n", BENCHMARK_N, BENCHMARK_MICRO_REPEATS, et1 - st1);

    return 0;
}
