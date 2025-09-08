#include "utils.h"
#include <stdio.h>
#include <string.h>

char a[BENCHMARK_N];

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

#if (BENCHMARK_N > 32768)
    ull st1 = get_cpu_time();
    memset(a, 0, sizeof(a));
    ull et1 = get_cpu_time();
    DoNotOptimize(a[0]);

    printf("misc.memset.cold_0:\t%d %llu\n", BENCHMARK_N, et1 - st1);
#else
    ull st1 = get_tsc();
    memset(a, 0, sizeof(a));
    ull et1 = get_tsc();
    DoNotOptimize(a[0]);

    printf("misc.memset.cold_0:\t%d %.10f\n", BENCHMARK_N, tsc_to_ns(et1 - st1));
#endif

    ull st2 = get_cpu_time();
    for (int i = 0; i < BENCHMARK_MICRO_REPEATS; ++i) {
        memset(a, 0, sizeof(a));
        DoNotOptimize(a[0]);
    }
    ull et2 = get_cpu_time();

    printf("misc.memset.hot_0:\t%d %.10f\n", BENCHMARK_N, (double)(et2 - st2) / BENCHMARK_MICRO_REPEATS);
    return 0;
}