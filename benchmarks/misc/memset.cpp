#include "utils.h"
#include <stdio.h>
#include <string.h>

char a[BENCHMARK_N];

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    measurement_handle_t st1 = start_measurement();
    memset(a, 0, sizeof(a));
    DoNotOptimize(a[0]);
    end_measurement(st1, "misc.memset.cold_0", 1);

    measurement_handle_t st2 = start_measurement();
    for (int i = 0; i < BENCHMARK_MICRO_REPEATS; ++i) {
        memset(a, 0, sizeof(a));
        DoNotOptimize(a[0]);
    }
    end_measurement(st2, "misc.memset.hot_0", BENCHMARK_MICRO_REPEATS);
    return 0;
}
