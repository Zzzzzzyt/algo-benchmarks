#include "utils.h"
#include <stdio.h>
#include <string.h>

char a[BENCHMARK_N];

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    // Fill buffer with nonzero chars and null-terminate
    for (int i = 0; i < BENCHMARK_N - 1; ++i)
        a[i] = 'a';
    a[BENCHMARK_N - 1] = '\0';

    measurement_handle_t st1 = start_measurement();
    for (int i = 0; i < BENCHMARK_MICRO_REPEATS; ++i) {
        volatile size_t len = strlen(a);
        DoNotOptimize(len);
    }
    end_measurement(st1, "misc.strlen", BENCHMARK_MICRO_REPEATS);

    return 0;
}
