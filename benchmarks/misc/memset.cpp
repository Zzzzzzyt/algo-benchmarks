#include "utils.h"
#include <string.h>

char buf[BENCHMARK_N];

int main(int argc, char *argv[]) {
    unsigned long long st1 = get_cpu_time();
    for (int i = 0; i < BENCHMARK_MICRO_REPEATS; ++i) {
        memset(buf, 0, sizeof(buf));
        DoNotOptimize(buf[0]);
    }
    unsigned long long et1 = get_cpu_time();

    unsigned long long st2 = get_cpu_time();
    for (int i = 0; i < BENCHMARK_MICRO_REPEATS; ++i) {
        memset(buf, 0, sizeof(buf));
        DoNotOptimize(buf[0]);
    }
    unsigned long long et2 = get_cpu_time();

    printf("misc.memset.cold_0:\t%d %d %llu\n", BENCHMARK_N, BENCHMARK_MICRO_REPEATS, et1 - st1);
    printf("misc.memset.hot_0:\t%d %d %llu\n", BENCHMARK_N, BENCHMARK_MICRO_REPEATS, et2 - st2);

    return 0;
}