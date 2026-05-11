#include "utils.h"
#include <algorithm>
#include <vector>

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    {
        std::vector<int> vec;
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            vec.push_back(i);
        }
        end_measurement(st, "container.vector.int_push_back", 1);
        int back = vec.back();
        DoNotOptimize(back);
    }

    {
        std::vector<int> vec;
        vec.reserve(BENCHMARK_N);
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            vec.push_back(i);
        }
        end_measurement(st, "container.vector.int_push_back_reserved", 1);
        int back = vec.back();
        DoNotOptimize(back);
    }

    {
        std::vector<int> shuffle_box = make_shuffled_ints(BENCHMARK_N);
        std::vector<int> vec(BENCHMARK_N);
        for (int i = 0; i < BENCHMARK_N; ++i) {
            vec[i] = i;
        }
        measurement_handle_t st = start_measurement();
        for (int value : vec) {
            DoNotOptimize(value);
        }
        end_measurement(st, "container.vector.int_iteration", 1);

        int sum = 0;
        measurement_handle_t st2 = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            DoNotOptimize(vec[shuffle_box[i]]);
        }
        end_measurement(st2, "container.vector.int_random_read", 1);
    }

    return 0;
}
