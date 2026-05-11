#include "utils.h"
#include <algorithm>
#include <queue>
#include <vector>

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    std::vector<int> values = make_shuffled_ints(BENCHMARK_N);

    {
        std::priority_queue<int> pq;
        measurement_handle_t st = start_measurement();
        for (int i = 0; i < BENCHMARK_N; ++i) {
            pq.push(values[i]);
        }
        end_measurement(st, "container.priority_queue.int_push", 1);
        int top = pq.top();
        DoNotOptimize(top);
    }

    {
        std::priority_queue<int> pq;
        for (int i = 0; i < BENCHMARK_N; ++i) {
            pq.push(values[i]);
        }
        measurement_handle_t st = start_measurement();
        while (!pq.empty()) {
            int top = pq.top();
            DoNotOptimize(top);
            pq.pop();
        }
        end_measurement(st, "container.priority_queue.int_pop", 1);
    }

    return 0;
}
