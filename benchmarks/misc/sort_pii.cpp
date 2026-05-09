#include "utils.h"
#include <algorithm>
#include <stdio.h>
#include <utility>

std::pair<int, int> a[BENCHMARK_N];

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = std::make_pair(rng(), rng());
    }

    measurement_handle_t st = start_measurement();
    std::sort(a, a + BENCHMARK_N);

    DoNotOptimize(a[0]);
    end_measurement(st, "misc.sort.pii_random", 1);

    return 0;
}
