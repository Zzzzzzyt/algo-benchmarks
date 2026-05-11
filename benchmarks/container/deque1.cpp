#include "utils.h"
#include <deque>
#include <vector>

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    {
        std::deque<int> dq;
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            dq.push_back(i);
        }
        end_measurement(st, "container.deque.int_push_back_cold", 1);
        int back = dq.back();
        DoNotOptimize(back);
    }

    {
        std::deque<int> dq;
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            dq.push_back(i);
        }
        end_measurement(st, "container.deque.int_push_back_hot", 1);
        int back = dq.back();
        DoNotOptimize(back);

        measurement_handle_t st2 = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            int back = dq.back();
            DoNotOptimize(back);
            dq.pop_back();
        }
        end_measurement(st2, "container.deque.int_pop_back", 1);
    }

    {
        std::deque<int> dq;
        for (int i = 0; i < BENCHMARK_N; ++i) {
            dq.push_front(i);
        }
        measurement_handle_t st = start_measurement();
        for (int value : dq) {
            DoNotOptimize(value);
        }
        end_measurement(st, "container.deque.int_iteration", 1);

        std::vector<int> shuffle_box = make_shuffled_ints(BENCHMARK_N);
        measurement_handle_t st2 = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            DoNotOptimize(dq[shuffle_box[i]]);
        }
        end_measurement(st2, "container.deque.int_random_read", 1);
    }

    return 0;
}
