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
#include "StealThreadPool.h"
#include "MQThreadPool.h"
#include "MQStealThreadPool.h"

namespace hku {

typedef std::pair<size_t, size_t> range_t;

inline std::vector<range_t> parallelIndexRange(size_t start, size_t end) {
    std::vector<std::pair<size_t, size_t>> ret;
    if (start >= end) {
        return ret;
    }

    size_t total = end - start;
    if (total == 0) {
        return ret;
    }

    size_t cpu_num = std::thread::hardware_concurrency();
    if (cpu_num == 1) {
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

template <typename FunctionType, class TaskGroup = MQStealThreadPool>
void parallel_for_index_void(size_t start, size_t end, FunctionType f) {
    auto ranges = parallelIndexRange(start, end);
    TaskGroup tg;
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

template <typename FunctionType, class TaskGroup = MQStealThreadPool>
auto parallel_for_index(size_t start, size_t end, FunctionType f) {
    auto ranges = parallelIndexRange(start, end);
    TaskGroup tg;
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

    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    for (auto& task : tasks) {
        auto one = task.get();
        for (auto&& value : one) {
            ret.emplace_back(std::move(value));
        }
    }

    return ret;
}

template <typename FunctionType, class TaskGroup = MQStealThreadPool>
auto parallel_for_range(size_t start, size_t end, FunctionType f) {
    auto ranges = parallelIndexRange(start, end);
    TaskGroup tg;
    std::vector<std::future<typename std::invoke_result<FunctionType, range_t>::type>> tasks;
    for (size_t i = 0, total = ranges.size(); i < total; i++) {
        tasks.emplace_back(tg.submit([func = f, range = ranges[i]]() { return func(range); }));
    }

    typename std::invoke_result<FunctionType, range_t>::type ret;
    for (auto& task : tasks) {
        auto one = task.get();
        for (auto&& value : one) {
            ret.emplace_back(std::move(value));
        }
    }

    return ret;
}

}  // namespace hku