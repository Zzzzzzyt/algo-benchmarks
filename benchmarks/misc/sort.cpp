#include "utils.h"
#include <algorithm>
#include <stdio.h>

int a[BENCHMARK_N];

int main(int argc, char *argv[]) {
    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = i;
    }

    ull st1 = get_cpu_time();
    std::sort(a, a + BENCHMARK_N);
    ull et1 = get_cpu_time();

    DoNotOptimize(a[0]);

    printf("misc.sort.int_sorted:\t%d %llu\n", BENCHMARK_N, et1 - st1);

    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = BENCHMARK_N - i;
    }

    ull st2 = get_cpu_time();
    std::sort(a, a + BENCHMARK_N);
    ull et2 = get_cpu_time();

    DoNotOptimize(a[0]);

    printf("misc.sort.int_reversed:\t%d %llu\n", BENCHMARK_N, et2 - st2);

    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = rng();
    }

    ull st3 = get_cpu_time();
    std::sort(a, a + BENCHMARK_N);
    ull et3 = get_cpu_time();

    DoNotOptimize(a[0]);

    printf("misc.sort.int_random:\t%d %llu\n", BENCHMARK_N, et3 - st3);

    return 0;
}
