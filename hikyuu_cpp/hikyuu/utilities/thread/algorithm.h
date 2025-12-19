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

//----------------------------------------------------------------
// Note: 除 ThreadPool/MQThreadPool 外，其他线程池由于使用
//       了 thread_local，本质为全局变量，只适合全局单例的方式使用,
//       否则会出现不同线程池示例互相影响导致出错。
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

}  // namespace hku