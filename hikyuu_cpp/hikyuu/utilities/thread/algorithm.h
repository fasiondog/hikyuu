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
//       如果都是纯计算(IO较少)，为防止嵌套情况下出现线程数量暴涨，
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
// 创建全局任务偷取线程池，主要目的用于计算并行，不适合带IO的计算
// 和前面 parallel_for 系列函数比较，主要目的防止嵌套并行时，
// 出现线程数量暴涨的情况。
// 前面 parallel_for 系列每次都会创建独立线程池。
// note: 程序内全局，初始化一次即可，重复初始化被忽略
//----------------------------------------------------------------
void HKU_UTILS_API init_global_task_group(size_t work_num = 0);

void HKU_UTILS_API release_global_task_group();

GlobalStealThreadPool* HKU_UTILS_API get_global_task_group();

size_t HKU_UTILS_API get_global_task_group_work_num();

// 辅助类，用于确保线程执行状态的正确管理
class HKU_UTILS_API ExecutionGuard {
private:
    static thread_local bool in_parallel_execution;
    bool* p_flag;

public:
    explicit ExecutionGuard(bool initial_value = true) : p_flag(&in_parallel_execution) {
        *p_flag = initial_value;
    }

    ~ExecutionGuard() {
        if (p_flag) {
            *p_flag = false;
        }
    }

    ExecutionGuard(const ExecutionGuard&) = delete;
    ExecutionGuard& operator=(const ExecutionGuard&) = delete;

    ExecutionGuard(ExecutionGuard&& other) : p_flag(other.p_flag) {
        other.p_flag = nullptr;
    }

    static bool is_executing() {
        return in_parallel_execution;
    }
};

template <typename FunctionType>
auto global_parallel_for_index_void(size_t start, size_t end, FunctionType f) {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if (ExecutionGuard::is_executing()) {
        // 当前线程已经在并行执行环境中，降级为串行执行避免死锁
        for (size_t i = start; i < end; i++) {
            f(i);
        }
        return;
    }

    ExecutionGuard guard;

    size_t work_num = tg->worker_num();
    auto ranges = parallelIndexRange(start, end, work_num);
    if (ranges.empty()) {
        return;
    }

    std::vector<std::future<void>> tasks;
    for (size_t i = 0, total = ranges.size(); i < total; i++) {
        tasks.emplace_back(tg->submit([func = f, range = ranges[i]]() {
            // 在任务内部也要检查嵌套
            if (ExecutionGuard::is_executing()) {
                // 如果在任务内部检测到嵌套，直接串行执行
                for (size_t ix = range.first; ix < range.second; ix++) {
                    func(ix);
                }
            } else {
                // 否则使用ExecutionGuard来标记嵌套状态
                ExecutionGuard guard_inner;
                for (size_t ix = range.first; ix < range.second; ix++) {
                    func(ix);
                }
            }
        }));
    }

    for (auto& task : tasks) {
        task.get();
    }

    return;
}

template <typename FunctionType>
auto global_parallel_for_index(size_t start, size_t end, FunctionType f) {
    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if (ExecutionGuard::is_executing()) {
        // 当前线程已经在并行执行环境中，降级为串行执行避免死锁
        for (size_t i = start; i < end; i++) {
            ret.emplace_back(f(i));
        }
        return ret;
    }

    // 使用RAII确保标志位正确清理
    ExecutionGuard guard;

    size_t work_num = tg->worker_num();
    auto ranges = parallelIndexRange(start, end, work_num);
    if (ranges.empty()) {
        return ret;
    }

    std::vector<std::future<std::vector<typename std::invoke_result<FunctionType, size_t>::type>>>
      tasks;
    for (size_t i = 0, total = ranges.size(); i < total; i++) {
        tasks.emplace_back(tg->submit([func = f, range = ranges[i]]() {
            // 在任务内部也要检查嵌套
            if (ExecutionGuard::is_executing()) {
                // 如果在任务内部检测到嵌套，直接串行执行
                std::vector<typename std::invoke_result<FunctionType, size_t>::type> one_ret;
                for (size_t ix = range.first; ix < range.second; ix++) {
                    one_ret.emplace_back(func(ix));
                }
                return one_ret;
            } else {
                // 否则使用ExecutionGuard来标记嵌套状态
                ExecutionGuard guard_inner;
                std::vector<typename std::invoke_result<FunctionType, size_t>::type> one_ret;
                for (size_t ix = range.first; ix < range.second; ix++) {
                    one_ret.emplace_back(func(ix));
                }
                return one_ret;
            }
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

template <typename FunctionType>
auto global_parallel_for_range(size_t start, size_t end, FunctionType f) {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");

    typename std::invoke_result<FunctionType, range_t>::type ret;
    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if (ExecutionGuard::is_executing()) {
        // 当前线程已经在并行执行环境中，降级为串行执行避免死锁
        auto ranges = parallelIndexRange(start, end, tg->worker_num());
        for (size_t i = 0, total = ranges.size(); i < total; i++) {
            auto one = f(ranges[i]);
            for (auto&& value : one) {
                ret.emplace_back(std::move(value));
            }
        }
        return ret;
    }

    ExecutionGuard guard;

    size_t work_num = tg->worker_num();
    auto ranges = parallelIndexRange(start, end, work_num);
    if (ranges.empty()) {
        return ret;
    }

    std::vector<std::future<typename std::invoke_result<FunctionType, range_t>::type>> tasks;
    for (size_t i = 0, total = ranges.size(); i < total; i++) {
        tasks.emplace_back(tg->submit([func = f, range = ranges[i]]() ->
                                      typename std::invoke_result<FunctionType, range_t>::type {
                                          // 在任务内部也要检查嵌套
                                          if (ExecutionGuard::is_executing()) {
                                              // 如果在任务内部检测到嵌套，直接串行执行
                                              return func(range);
                                          } else {
                                              // 否则使用ExecutionGuard来标记嵌套状态
                                              ExecutionGuard guard_inner;
                                              return func(range);
                                          }
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

template <typename FunctionType>
void global_parallel_for_index_void_single(size_t start, size_t end, FunctionType f) {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if (ExecutionGuard::is_executing()) {
        // 当前线程已经在并行执行环境中，降级为串行执行避免死锁
        HKU_INFO("Current thread is already in parallel execution environment");
        for (size_t i = start; i < end; i++) {
            f(i);
        }
        return;
    }

    // 使用RAII确保标志位正确清理
    ExecutionGuard guard;

    if (start >= end) {
        return;
    }

    std::vector<std::future<void>> tasks;
    for (size_t i = start; i < end; i++) {
        tasks.push_back(tg->submit([func = f, i]() {
            if (ExecutionGuard::is_executing()) {
                func(i);
            } else {
                ExecutionGuard guard_inner;
                func(i);
            }
        }));
    }

    for (auto& task : tasks) {
        task.get();
    }
    return;
}

template <typename FunctionType>
auto global_parallel_for_index_single(size_t start, size_t end, FunctionType f) {
    auto* tg = get_global_task_group();
    HKU_CHECK(tg, "Global task group is not initialized!");

    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if (ExecutionGuard::is_executing()) {
        // 当前线程已经在并行执行环境中，降级为串行执行避免死锁
        for (size_t i = start; i < end; i++) {
            ret.push_back(f(i));
        }
        return ret;
    }

    // 使用RAII确保标志位正确清理
    ExecutionGuard guard;

    if (start >= end) {
        return ret;
    }

    std::vector<std::future<typename std::invoke_result<FunctionType, size_t>::type>> tasks;
    for (size_t i = start; i < end; i++) {
        tasks.emplace_back(
          tg->submit([func = f, i]() -> typename std::invoke_result<FunctionType, size_t>::type {
              // 在任务内部也要检查嵌套
              if (ExecutionGuard::is_executing()) {
                  return func(i);
              } else {
                  ExecutionGuard guard_inner;
                  return func(i);
              }
          }));
    }

    for (auto& task : tasks) {
        ret.push_back(std::move(task.get()));
    }

    return ret;
}

}  // namespace hku