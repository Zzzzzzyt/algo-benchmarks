#include "utils.h"
#include <deque>
#include <vector>

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    {
        std::deque<int> dq;
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            dq.push_front(i);
        }
        end_measurement(st, "container.deque.int_push_front_cold", 1);
        int front = dq.front();
        DoNotOptimize(front);
    }

    {
        std::deque<int> dq;
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            dq.push_front(i);
        }
        end_measurement(st, "container.deque.int_push_front_hot", 1);
        int front = dq.front();
        DoNotOptimize(front);

        measurement_handle_t st2 = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            int front = dq.front();
            DoNotOptimize(front);
            dq.pop_front();
        }
        end_measurement(st2, "container.deque.int_pop_front", 1);
    }

    return 0;
}
