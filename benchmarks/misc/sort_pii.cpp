#include "utils.h"
#include <algorithm>
#include <stdio.h>
#include <utility>

std::pair<int, int> a[BENCHMARK_N];

int main(int argc, char *argv[]) {
    for (int i = 0; i < BENCHMARK_N; i++) {
        a[i] = std::make_pair(rng(), rng());
    }

    unsigned long long st = get_cpu_time();
    std::sort(a, a + BENCHMARK_N);
    unsigned long long et = get_cpu_time();

    DoNotOptimize(a[0]);

    printf("misc.sort.pii_random:\t%d %llu\n", BENCHMARK_N, et - st);

    return 0;
}
