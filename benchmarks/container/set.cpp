#include "utils.h"
#include <algorithm>
#include <set>
#include <vector>

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    std::vector<int> values = make_shuffled_ints(BENCHMARK_N);

    {
        std::set<int> stl_set;
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            stl_set.insert(values[i]);
        }
        size_t sz = stl_set.size();
        DoNotOptimize(sz);
        end_measurement(st, "container.set.int_random_insert", 1);
    }

    {
        std::set<int> stl_set;
        for (int i = 0; i < BENCHMARK_N; ++i) {
            stl_set.insert(values[i]);
        }
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            std::set<int>::iterator it = stl_set.find(values[i]);
            if (it != stl_set.end()) {
                int value = *it;
                DoNotOptimize(value);
            }
        }
        end_measurement(st, "container.set.int_random_find_hit", 1);
    }

    {
        std::set<int> stl_set;
        for (int i = 0; i < BENCHMARK_N; ++i) {
            stl_set.insert(values[i]);
        }
        measurement_handle_t st = start_measurement();
        for (int value : stl_set) {
            DoNotOptimize(value);
        }
        end_measurement(st, "container.set.int_iteration", 1);
    }

    return 0;
}
