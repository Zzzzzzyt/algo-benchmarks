#include "utils.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>

int a[BENCHMARK_N];

int main(int argc, char *argv[]) {
    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = i;
    }

    unsigned long long st1 = get_cpu_time();
    std::sort(a, a + BENCHMARK_N);
    unsigned long long et1 = get_cpu_time();

    DoNotOptimize(a[0]);

    printf("misc.sort.sorted:\t%d %llu\n", BENCHMARK_N, et1 - st1);

    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = BENCHMARK_N - i;
    }

    unsigned long long st2 = get_cpu_time();
    std::sort(a, a + BENCHMARK_N);
    unsigned long long et2 = get_cpu_time();

    DoNotOptimize(a[0]);

    printf("misc.sort.reversed:\t%d %llu\n", BENCHMARK_N, et2 - st2);

    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = rng();
    }

    unsigned long long st3 = get_cpu_time();
    std::sort(a, a + BENCHMARK_N);
    unsigned long long et3 = get_cpu_time();

    DoNotOptimize(a[0]);

    printf("misc.sort.random:\t%d %llu\n", BENCHMARK_N, et3 - st3);

    return 0;
}
