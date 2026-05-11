#include "utils.h"
#include <algorithm>
#include <unordered_set>
#include <vector>

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    std::vector<int> values = make_shuffled_ints(BENCHMARK_N);

    {
        std::unordered_set<int> stl_set;
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            stl_set.insert(values[i]);
        }
        size_t sz = stl_set.size();
        DoNotOptimize(sz);
        end_measurement(st, "container.unordered_set.int_insert", 1);
    }

    {
        std::unordered_set<int> stl_set;
        stl_set.reserve(BENCHMARK_N);
        for (int i = 0; i < BENCHMARK_N; ++i) {
            stl_set.insert(values[i]);
        }
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            std::unordered_set<int>::iterator it = stl_set.find(values[i]);
            if (it != stl_set.end()) {
                int value = *it;
                DoNotOptimize(value);
            }
        }
        end_measurement(st, "container.unordered_set.int_find_hit", 1);
    }

    {
        std::unordered_set<int> stl_set;
        stl_set.reserve(BENCHMARK_N);
        for (int i = 0; i < BENCHMARK_N; ++i) {
            stl_set.insert(values[i]);
        }
        measurement_handle_t st = start_measurement();
        for (int value : stl_set) {
            DoNotOptimize(value);
        }
        end_measurement(st, "container.unordered_set.int_iteration", 1);
    }

    return 0;
}
