#include "utils.h"
#include <algorithm>
#include <stdio.h>

int a[BENCHMARK_N];

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = i;
    }

    measurement_handle_t st1 = start_measurement();
    std::sort(a, a + BENCHMARK_N);

    DoNotOptimize(a[0]);
    end_measurement(st1, "misc.sort.int_sorted", 1);

    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = BENCHMARK_N - i;
    }

    measurement_handle_t st2 = start_measurement();
    std::sort(a, a + BENCHMARK_N);

    DoNotOptimize(a[0]);
    end_measurement(st2, "misc.sort.int_reversed", 1);

    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = rng();
    }

    measurement_handle_t st3 = start_measurement();
    std::sort(a, a + BENCHMARK_N);

    DoNotOptimize(a[0]);
    end_measurement(st3, "misc.sort.int_random", 1);

    return 0;
}
