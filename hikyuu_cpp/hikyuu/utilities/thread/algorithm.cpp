/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-01-16
 *      Author: fasiondog
 */

#include <memory>
#include "algorithm.h"

namespace hku {

static std::unique_ptr<GlobalStealThreadPool> global_steal_thread_pool;

thread_local bool ExecutionGuard::in_parallel_execution = false;

void HKU_UTILS_API init_global_task_group(size_t work_num) {
    size_t cpu_num = std::thread::hardware_concurrency();
    if (work_num == 0) {
        work_num = cpu_num;
    }
    if (!global_steal_thread_pool) {
        global_steal_thread_pool = std::make_unique<GlobalStealThreadPool>(work_num, false);
    }
}

void HKU_UTILS_API release_global_task_group() {
    if (global_steal_thread_pool) {
        global_steal_thread_pool.reset();
    }
}

GlobalStealThreadPool *HKU_UTILS_API get_global_task_group() {
    return global_steal_thread_pool.get();
}

}  // namespace hku