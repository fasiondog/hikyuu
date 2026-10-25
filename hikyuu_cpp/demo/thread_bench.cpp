/*
 * Pure CPU benchmark for the global task pool, no data driver or network involved.
 *
 * usage: thread_bench <scenario> <outer> <inner> <work_per_item> <rounds>
 *   flat:   outer*inner tasks, each burns work_per_item cycles, no nested parallel
 *   nested: outer tasks, each submits an inner parallel of inner items (each burns work_per_item)
 */

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "hikyuu/utilities/thread/algorithm.h"

using namespace hku;

volatile double g_sink = 0.0;

static double burn(size_t count) {
    double sum = 0.0;
    for (size_t i = 0; i < count; i++) {
        sum += std::sqrt(static_cast<double>(i % 997 + 1));
    }
    return sum;
}

int main(int argc, char* argv[]) {
    init_global_task_group();

    const std::string scenario = argc > 1 ? argv[1] : "nested";
    const size_t outer = argc > 2 ? std::strtoul(argv[2], nullptr, 10) : 4000;
    const size_t inner = argc > 3 ? std::strtoul(argv[3], nullptr, 10) : 32;
    const size_t work_per_item = argc > 4 ? std::strtoul(argv[4], nullptr, 10) : 250;
    const size_t rounds = argc > 5 ? std::strtoul(argv[5], nullptr, 10) : 5;

    auto run_nested = [&]() {
        global_parallel_for_index_void_single(0, outer, [&](size_t) {
            auto parts =
              global_parallel_for_index(0, inner, [&](size_t) { return burn(work_per_item); });
            double sum = 0.0;
            for (double v : parts) {
                sum += v;
            }
            g_sink += sum;
        });
    };

    auto run_flat = [&]() {
        global_parallel_for_index_void_single(0, outer * inner,
                                              [&](size_t) { g_sink += burn(work_per_item); });
    };

    size_t total_stack = 0;
    size_t remaining_stack = hku::detail::remaining_stack_size(&total_stack);
    printf("main thread stack: total=%zu bytes, remaining=%zu bytes\n", total_stack,
           remaining_stack);
    printf("scenario=%s outer=%zu inner=%zu work=%zu\n", scenario.c_str(), outer, inner,
           work_per_item);
    for (size_t r = 0; r < rounds; r++) {
        auto t0 = std::chrono::steady_clock::now();
        if (scenario == "flat") {
            run_flat();
        } else {
            run_nested();
        }
        auto t1 = std::chrono::steady_clock::now();
        printf("round %zu: %.1f ms\n", r + 1,
               std::chrono::duration<double, std::milli>(t1 - t0).count());
    }

    release_global_task_group();
    return 0;
}
