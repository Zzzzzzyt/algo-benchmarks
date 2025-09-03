#include "utils.h"
#include <string.h>

char buf[BENCHMARK_N];

int main(int argc, char *argv[]) {
    // Fill buffer with nonzero chars and null-terminate
    for (int i = 0; i < BENCHMARK_N - 1; ++i) buf[i] = 'a';
    buf[BENCHMARK_N - 1] = '\0';

    unsigned long long st1 = get_cpu_time();
    for (int i = 0; i < BENCHMARK_MICRO_REPEATS; ++i) {
        volatile size_t len = strlen(buf);
        DoNotOptimize(len);
    }
    unsigned long long et1 = get_cpu_time();

    unsigned long long st2 = get_cpu_time();
    for (int i = 0; i < BENCHMARK_MICRO_REPEATS; ++i) {
        volatile size_t len = strlen(buf);
        DoNotOptimize(len);
    }
    unsigned long long et2 = get_cpu_time();

    printf("misc.strlen.cold_0:\t%d %d %llu\n", BENCHMARK_N, BENCHMARK_MICRO_REPEATS, et1 - st1);
    printf("misc.strlen.hot_0:\t%d %d %llu\n", BENCHMARK_N, BENCHMARK_MICRO_REPEATS, et2 - st2);

    return 0;
}
