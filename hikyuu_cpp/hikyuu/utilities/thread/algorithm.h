/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-27
 *      Author: fasiondog
 */

#pragma once

#include <future>
#include <functional>
#include <vector>
#include <limits>
#include "ThreadPool.h"
#include "MQThreadPool.h"
#include "StealThreadPool.h"
#include "MQStealThreadPool.h"
#include "GlobalMQThreadPool.h"
#include "GlobalStealThreadPool.h"
#include "GlobalMQStealThreadPool.h"
#include "GlobalThreadPool.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

//----------------------------------------------------------------
// Note: 除 ThreadPool/MQThreadPool 外，其他线程池由于使用
//       了 thread_local，本质为全局变量，只适合全局单例的方式使用,
//       否则会出现不同线程池示例互相影响导致出错。
//       每次会创建独立的线程池计算。
//       如果都是纯计算(IO较少)，
//       建议创建全局线程池，并使用全局线程池进行计算。
//----------------------------------------------------------------

namespace hku {

typedef std::pair<size_t, size_t> range_t;

inline std::vector<range_t> parallelIndexRange(size_t start, size_t end, size_t cpu_num = 0) {
    std::vector<std::pair<size_t, size_t>> ret;
    if (start >= end) {
        return ret;
    }

    size_t total = end - start;
    if (cpu_num == 0) {
        cpu_num = std::thread::hardware_concurrency();
    }
    if (cpu_num <= 1) {
        ret.emplace_back(start, end);
        return ret;
    }

    size_t per_num = total / cpu_num;
    if (per_num > 0) {
        for (size_t i = 0; i < cpu_num; i++) {
            size_t first = i * per_num + start;
            ret.emplace_back(first, first + per_num);
        }
    }

    for (size_t i = per_num * cpu_num + start; i < end; i++) {
        ret.emplace_back(i, i + 1);
    }

    return ret;
}

template <typename FunctionType, class TaskGroup = MQThreadPool>
void parallel_for_index_void(size_t start, size_t end, FunctionType f, int cpu_num = 0) {
    auto ranges = parallelIndexRange(start, end, cpu_num);
    if (ranges.empty()) {
        return;
    }

    TaskGroup tg(cpu_num == 0 ? std::thread::hardware_concurrency() : cpu_num);
    for (size_t i = 0, total = ranges.size(); i < total; i++) {
        tg.submit([=, range = ranges[i]]() {
            for (size_t ix = range.first; ix < range.second; ix++) {
                f(ix);
            }
        });
    }
    tg.join();
    return;
}

template <typename FunctionType, class TaskGroup = MQThreadPool>
auto parallel_for_index(size_t start, size_t end, FunctionType f, size_t cpu_num = 0) {
    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    auto ranges = parallelIndexRange(start, end, cpu_num);
    if (ranges.empty()) {
        return ret;
    }

    TaskGroup tg(cpu_num == 0 ? std::thread::hardware_concurrency() : cpu_num);
    std::vector<std::future<std::vector<typename std::invoke_result<FunctionType, size_t>::type>>>
      tasks;
    for (size_t i = 0, total = ranges.size(); i < total; i++) {
        tasks.emplace_back(tg.submit([func = f, range = ranges[i]]() {
            std::vector<typename std::invoke_result<FunctionType, size_t>::type> one_ret;
            for (size_t ix = range.first; ix < range.second; ix++) {
                one_ret.emplace_back(func(ix));
            }
            return one_ret;
        }));
    }

    for (auto& task : tasks) {
        auto one = task.get();
        for (auto&& value : one) {
            ret.emplace_back(std::move(value));
        }
    }

    return ret;
}

template <typename FunctionType, class TaskGroup = MQThreadPool>
auto parallel_for_range(size_t start, size_t end, FunctionType f, size_t cpu_num = 0) {
    typename std::invoke_result<FunctionType, range_t>::type ret;
    auto ranges = parallelIndexRange(start, end, cpu_num);
    if (ranges.empty()) {
        return ret;
    }

    TaskGroup tg(cpu_num == 0 ? std::thread::hardware_concurrency() : cpu_num);
    std::vector<std::future<typename std::invoke_result<FunctionType, range_t>::type>> tasks;
    for (size_t i = 0, total = ranges.size(); i < total; i++) {
        tasks.emplace_back(tg.submit([func = f, range = ranges[i]]() { return func(range); }));
    }

    for (auto& task : tasks) {
        auto one = task.get();
        for (auto&& value : one) {
            ret.emplace_back(std::move(value));
        }
    }

    return ret;
}

template <typename FunctionType, class TaskGroup = ThreadPool>
void parallel_for_index_void_single(size_t start, size_t end, FunctionType f, int cpu_num = 0) {
    if (start >= end) {
        return;
    }

    TaskGroup tg(cpu_num == 0 ? std::thread::hardware_concurrency() : cpu_num);
    for (size_t i = start; i < end; i++) {
        tg.submit([func = f, i]() { func(i); });
    }
    tg.join();
    return;
}

template <typename FunctionType, class TaskGroup = ThreadPool>
auto parallel_for_index_single(size_t start, size_t end, FunctionType f, size_t cpu_num = 0) {
    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    if (start >= end) {
        return ret;
    }

    TaskGroup tg(cpu_num == 0 ? std::thread::hardware_concurrency() : cpu_num);
    std::vector<std::future<typename std::invoke_result<FunctionType, size_t>::type>> tasks;
    for (size_t i = start; i < end; i++) {
        tasks.emplace_back(tg.submit([func = f, i]() { return func(i); }));
    }

    for (auto& task : tasks) {
        ret.push_back(std::move(task.get()));
    }

    return ret;
}

//----------------------------------------------------------------
// 创建全局任务偷取线程池，主要目的用于计算密集或少量混合IO的并行，不适合纯IO的并行
// 前面 parallel_for 系列每次都会创建独立线程池。
// note: 程序内全局，初始化一次即可，重复初始化被忽略
//----------------------------------------------------------------
void HKU_UTILS_API init_global_task_group(size_t work_num = 0);

void HKU_UTILS_API release_global_task_group();

HKU_UTILS_API GlobalStealThreadPool* get_global_task_group();

size_t HKU_UTILS_API get_global_task_group_work_num();

template <typename FutureContainer>
void wait_for_all_non_blocking(GlobalStealThreadPool& pool, FutureContainer& futures) {
    // 如果当前线程是工作线程，其子任务加入的是自身队列前端，其他线程无法获取子任务，需要唤醒
    // 非工作线程时，其子任务加入的时主队列，无需主动唤醒
    if (pool.is_work_thread()) {
        pool.wake_up();
    }

    bool all_ready = false;
    auto init_delay = std::chrono::microseconds(1);
    auto delay = init_delay;
    const auto max_delay = std::chrono::microseconds(50000);

    while (!all_ready && !pool.done()) {
        all_ready = true;
        for (auto& future : futures) {
            if (future.wait_for(std::chrono::nanoseconds(0)) != std::future_status::ready) {
                all_ready = false;
                break;
            }
        }

        // 如果不是所有任务都完成，尝试执行本地任务
        if (!all_ready) {
            if (pool.run_available_task_once()) {
                delay = init_delay;
            } else if (pool.done()) {
                break;
            } else {
                std::this_thread::sleep_for(delay);
                if (delay < max_delay) {
                    delay = std::min(delay * 2, max_delay);
                }
            }
        }
    }
}

/** 使用global_submit_task提交的任务，必须使用global_wait_task，global_wake_up 配合 */
template <typename FunctionType>
auto global_submit_task(FunctionType f, bool enable_nested = true) {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");
    return tg->submit(f);
}

inline void global_wake_up() {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");
    if (tg->is_work_thread()) {
        tg->wake_up();
    }
}

template <typename FutureType>
void global_wait_task(FutureType& future) {
    auto* tg = get_global_task_group();
    bool ready = false;
    auto init_delay = std::chrono::microseconds(1);
    auto delay = init_delay;
    const auto max_delay = std::chrono::microseconds(50000);

    while (!ready && !tg->done()) {
        ready = true;
        if (future.wait_for(std::chrono::nanoseconds(0)) != std::future_status::ready) {
            ready = false;
        }

        // 如果任务未完成，尝试执行本地任务
        if (!ready) {
            if (tg->run_available_task_once()) {
                delay = init_delay;
            } else if (tg->done()) {
                break;
            } else {
                std::this_thread::sleep_for(delay);
                if (delay < max_delay) {
                    delay = std::min(delay * 2, max_delay);
                }
            }
        }
    }
}

template <typename FunctionType>
auto global_parallel_for_index_void(size_t start, size_t end, FunctionType f, size_t threshold = 2,
                                    bool enable_nested = true) {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");
    HKU_IF_RETURN(start >= end, void());

    // 如果任务数量小于阈值，或者当前是工作线程且禁止嵌套, 则直接执行
    if ((end - start) < threshold || (!enable_nested && tg->is_work_thread())) {
        for (size_t i = start; i < end; i++) {
            f(i);
        }
        return;
    }

    auto ranges = parallelIndexRange(start, end, tg->worker_num());
    if (ranges.empty()) {
        return;
    }

    std::vector<std::future<void>> tasks;
    tasks.reserve(ranges.size());
    for (size_t i = 0, total = ranges.size(); i < total; i++) {
        tasks.emplace_back(tg->submit([func = f, range = ranges[i]]() {
            for (size_t ix = range.first; ix < range.second; ix++) {
                func(ix);
            }
        }));
    }

    wait_for_all_non_blocking(*tg, tasks);

    for (auto& task : tasks) {
        task.get();
    }

    return;
}

template <typename FunctionType>
auto global_parallel_for_index(size_t start, size_t end, FunctionType f, size_t threshold = 2,
                               bool enable_nested = true) {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");

    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    HKU_IF_RETURN(start >= end, ret);

    ret.reserve(end - start);

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if ((end - start) < threshold || (!enable_nested && tg->is_work_thread())) {
        for (size_t i = start; i < end; i++) {
            ret.emplace_back(f(i));
        }
        return ret;
    }

    auto ranges = parallelIndexRange(start, end, tg->worker_num());
    if (ranges.empty()) {
        return ret;
    }

    std::vector<std::future<std::vector<typename std::invoke_result<FunctionType, size_t>::type>>>
      tasks;
    tasks.reserve(ranges.size());
    for (size_t i = 0, total = ranges.size(); i < total; i++) {
        tasks.emplace_back(tg->submit([func = f, range = ranges[i]]() {
            std::vector<typename std::invoke_result<FunctionType, size_t>::type> one_ret;
            one_ret.reserve(range.second - range.first);
            for (size_t ix = range.first; ix < range.second; ix++) {
                one_ret.emplace_back(func(ix));
            }
            return one_ret;
        }));
    }

    wait_for_all_non_blocking(*tg, tasks);

    for (auto& task : tasks) {
        auto one = task.get();
        for (auto&& value : one) {
            ret.emplace_back(std::move(value));
        }
    }

    return ret;
}

template <typename FunctionType>
void global_parallel_for_index_void_single(size_t start, size_t end, FunctionType f,
                                           size_t threshold = 1, bool enable_nested = true) {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");
    HKU_IF_RETURN(start >= end, void());

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if ((end - start) < threshold || (!enable_nested && tg->is_work_thread())) {
        for (size_t i = start; i < end; i++) {
            f(i);
        }
        return;
    }

    std::vector<std::future<void>> tasks;
    tasks.reserve(end - start);
    for (size_t i = start; i < end; i++) {
        tasks.push_back(tg->submit([func = f, i]() { func(i); }));
    }

    wait_for_all_non_blocking(*tg, tasks);

    for (auto& task : tasks) {
        task.get();
    }
    return;
}

template <typename FunctionType>
auto global_parallel_for_index_single(size_t start, size_t end, FunctionType f,
                                      size_t threshold = 1, bool enable_nested = true) {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");

    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    HKU_IF_RETURN(start >= end, ret);

    ret.reserve(end - start);

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if ((end - start) < threshold || (!enable_nested && tg->is_work_thread())) {
        for (size_t i = start; i < end; i++) {
            ret.push_back(f(i));
        }
        return ret;
    }

    std::vector<std::future<typename std::invoke_result<FunctionType, size_t>::type>> tasks;
    tasks.reserve(end - start);
    for (size_t i = start; i < end; i++) {
        tasks.emplace_back(
          tg->submit([func = f, i]() ->
                     typename std::invoke_result<FunctionType, size_t>::type { return func(i); }));
    }

    wait_for_all_non_blocking(*tg, tasks);

    for (auto& task : tasks) {
        ret.push_back(std::move(task.get()));
    }

    return ret;
}

}  // namespace hku