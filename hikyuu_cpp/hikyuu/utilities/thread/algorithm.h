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

#if CPP_STANDARD >= CPP_STANDARD_20
#include <boost/asio.hpp>
#include <type_traits>
#include <exception>
#endif

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
    bool is_work_thread = GlobalStealThreadPool::is_work_thread();
    if (is_work_thread) {
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

        // 如果不是所有任务都完成，尝试执行积累任务
        if (!all_ready) {
            if (!pool.run_available_task_once()) {
                return;
            }
            if (pool.run_available_task_once()) {
                delay = init_delay;
            } else if (pool.done()) {
                // 非工作线程也要参与忙等，否则在内存不足时更容易发生内存交换
                return;
            } else {
                // 工作线程休眠忙等
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
    if (GlobalStealThreadPool::is_work_thread()) {
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
    HKU_IF_RETURN(start >= end, void());

    // 如果任务数量小于阈值，或者当前是工作线程且禁止嵌套, 则直接执行
    if ((end - start) < threshold || (!enable_nested && GlobalStealThreadPool::is_work_thread())) {
        for (size_t i = start; i < end; i++) {
            f(i);
        }
        return;
    }

    auto* tg = get_global_task_group();
    HKU_ASSERT(tg);

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
    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    HKU_IF_RETURN(start >= end, ret);

    ret.reserve(end - start);

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if ((end - start) < threshold || (!enable_nested && GlobalStealThreadPool::is_work_thread())) {
        for (size_t i = start; i < end; i++) {
            ret.emplace_back(f(i));
        }
        return ret;
    }

    auto* tg = get_global_task_group();
    HKU_ASSERT(tg);

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
    HKU_IF_RETURN(start >= end, void());

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if ((end - start) < threshold || (!enable_nested && GlobalStealThreadPool::is_work_thread())) {
        for (size_t i = start; i < end; i++) {
            f(i);
        }
        return;
    }

    auto* tg = get_global_task_group();
    HKU_ASSERT(tg);

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
    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    HKU_IF_RETURN(start >= end, ret);

    ret.reserve(end - start);

    // 检查当前线程是否已经在执行某个任务，如果是则降级为串行执行
    if ((end - start) < threshold || (!enable_nested && GlobalStealThreadPool::is_work_thread())) {
        for (size_t i = start; i < end; i++) {
            ret.push_back(f(i));
        }
        return ret;
    }

    auto* tg = get_global_task_group();
    HKU_ASSERT(tg);

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

#if CPP_STANDARD >= CPP_STANDARD_20
//----------------------------------------------------------------
// 协程
//----------------------------------------------------------------
namespace asio = boost::asio;

/**
 * @brief 在协程中等待 std::future 的适配器函数
 *
 * 这是推荐的使用方式，用于在 boost::asio 协程中优雅地等待传统的 std::future。
 *
 * ## 使用场景
 *
 * ### 1. 包装现有的基于 future 的 API
 * @code
 *   // 假设有一个返回 std::future 的函数
 *   std::future<int> compute_async();
 *
 *   // 在协程中使用
 *   asio::awaitable<void> my_coroutine() {
 *       int result = co_await await_future(compute_async());
 *   }
 * @endcode
 *
 * ### 2. 在线程池中执行任务并在协程中等待
 * @code
 *   asio::awaitable<void> coroutine_with_pool() {
 *       ThreadPool pool(4);
 *
 *       // 提交任务获取 future
 *       auto fut = pool.submit([]() { return heavy_compute(); });
 *
 *       // 在协程中等待结果（不阻塞事件循环）
 *       int result = co_await await_future(std::move(fut));
 *   }
 * @endcode
 *
 * ### 3. 共享 future（多个协程等待同一个任务）
 * @code
 *   asio::awaitable<void> shared_future_example() {
 *       ThreadPool pool(4);
 *       auto fut_ptr = std::make_shared<std::future<int>>(pool.submit(task));
 *
 *       // 多个协程可以等待同一个任务
 *       int r1 = co_await await_future(fut_ptr);
 *       int r2 = co_await await_future(fut_ptr);
 *   }
 * @endcode
 *
 * ## 实现原理
 *
 * 使用 `asio::steady_timer` 进行高效轮询（100 微秒间隔），避免 busy wait：
 * 1. 定期检查 future 是否 ready
 * 2. 未就绪时通过 timer 异步挂起协程，释放执行权给事件循环
 * 3. future 就绪后立即恢复协程并返回结果
 *
 * ## 异常处理
 *
 * 如果 future 中包含异常，该异常会被重新抛出到协程中：
 * @code
 *   try {
 *       auto result = co_await await_future(std::move(fut));
 *   } catch (const std::exception& e) {
 *       // 处理 future 中的异常
 *   }
 * @endcode
 *
 * @tparam T future 的返回值类型
 * @param fut std::future<T> 对象（右值引用）
 * @return asio::awaitable<T> 可在协程中 co_await 的对象
 *
 * @see co_run - 标准的异步协程任务执行接口
 * @see co_dispatch_no_wait - fire-and-forget 模式
 */
template <typename T>
auto await_future(std::future<T> fut) -> asio::awaitable<T> {
    auto exec = co_await asio::this_coro::executor;
    asio::steady_timer timer(exec);

    // 使用 timer 进行高效轮询，避免 busy wait
    while (fut.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        timer.expires_after(std::chrono::microseconds(100));
        co_await timer.async_wait(asio::use_awaitable);
    }

    co_return fut.get();  // 可能抛出异常
}

/**
 * @brief void 特化版本
 *
 * 用于等待无返回值的 std::future<void> 对象。
 * 行为与模板版本相同，只是不返回值，主要用于等待任务完成。
 *
 * @param fut std::future<void> 对象
 * @return asio::awaitable<void> 可在协程中 co_await 的对象
 *
 * @example
 *   asio::awaitable<void> example() {
 *       ThreadPool pool(4);
 *       auto fut = pool.submit([]() {
 *           // 执行一些操作
 *           do_something();
 *       });
 *
 *       // 等待任务完成
 *       co_await await_future(std::move(fut));
 *   }
 */
template <>
inline auto await_future<void>(std::future<void> fut) -> asio::awaitable<void> {
    auto exec = co_await asio::this_coro::executor;
    asio::steady_timer timer(exec);

    while (fut.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        timer.expires_after(std::chrono::microseconds(100));
        co_await timer.async_wait(asio::use_awaitable);
    }

    fut.get();  // 可能抛出异常
}

/**
 * @brief shared_ptr 版本的 await_future（适用于需要共享 future 的场景）
 *
 * 当多个协程需要等待同一个异步任务的结果时使用。通过 std::shared_ptr 管理
 * std::future 的生命周期，允许多个协程安全地等待同一个任务。
 *
 * ## 使用场景
 *
 * ### 1. 广播模式 - 多个协程等待同一事件
 * @code
 *   asio::awaitable<void> broadcast_example() {
 *       ThreadPool pool(4);
 *       auto fut_ptr = std::make_shared<std::future<int>>(pool.submit([]() {
 *           return compute_expensive_value();
 *       }));
 *
 *       // 启动多个协程，都等待同一个计算结果
 *       co_spawn(co_await asio::this_coro::executor,
 *                [fut_ptr]() -> asio::awaitable<void> {
 *                    int result = co_await await_future(fut_ptr);
 *                    // 使用结果...
 *                }, asio::detached);
 *
 *       co_spawn(co_await asio::this_coro::executor,
 *                [fut_ptr]() -> asio::awaitable<void> {
 *                    int result = co_await await_future(fut_ptr);
 *                    // 使用结果...
 *                }, asio::detached);
 *   }
 * @endcode
 *
 * ### 2. 缓存异步结果
 * @code
 *   class DataCache {
 *   private:
 *       std::shared_ptr<std::future<std::string>> cached_data;
 *
 *   public:
 *       asio::awaitable<std::string> get_data() {
 *           if (!cached_data || cached_data->wait_for(std::chrono::seconds(0)) ==
 * std::future_status::ready) { auto promise = std::make_shared<std::promise<std::string>>();
 *               cached_data = std::make_shared<std::future<std::string>>(promise->get_future());
 *
 *               std::thread([promise]() {
 *                   try {
 *                       promise->set_value(fetch_from_network());
 *                   } catch (...) {
 *                       promise->set_exception(std::current_exception());
 *                   }
 *               }).detach();
 *           }
 *
 *           co_return co_await await_future(cached_data);
 *       }
 *   };
 * @endcode
 *
 * ## 注意事项
 *
 * 1. 所有等待同一个 shared_ptr 的协程会在 future 就绪时几乎同时恢复
 * 2. 异常处理：如果 future 包含异常，每个等待的协程都会收到相同的异常
 * 3. 性能：相比直接传递 future，shared_ptr 版本有轻微的性能开销
 *
 * @tparam T future 的返回值类型
 * @param fut_ptr std::shared_ptr<std::future<T>> 共享的 future 指针
 * @return asio::awaitable<T> 可在协程中 co_await 的对象
 *
 * @see await_future(std::future<T>) - 直接 future 版本
 * @see co_run - 标准的异步协程任务执行接口
 * @see co_dispatch_no_wait - fire-and-forget 模式
 */
template <typename T>
auto await_future(std::shared_ptr<std::future<T>> fut_ptr) -> asio::awaitable<T> {
    auto exec = co_await asio::this_coro::executor;
    asio::steady_timer timer(exec);

    while (fut_ptr->wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        timer.expires_after(std::chrono::microseconds(100));
        co_await timer.async_wait(asio::use_awaitable);
    }

    co_return fut_ptr->get();  // 可能抛出异常
}

/**
 * @brief void 特化版本（shared_ptr）
 *
 * shared_ptr 版本的 await_future 的 void 特化，用于等待无返回值的共享 future。
 * 主要用于多个协程需要同步等待某个异步操作完成的场景。
 *
 * @param fut_ptr std::shared_ptr<std::future<void>> 共享的 void future 指针
 * @return asio::awaitable<void> 可在协程中 co_await 的对象
 *
 * @example
 *   asio::awaitable<void> shared_void_example() {
 *       ThreadPool pool(4);
 *       auto event = std::make_shared<std::future<void>>(
 *           pool.submit([]() {
 *               // 初始化耗时操作
 *               initialize_system();
 *           })
 *       );
 *
 *       // 多个服务协程等待系统初始化完成
 *       co_await await_future(event);
 *       co_await await_future(event); // 另一个协程同样等待
 *   }
 */
template <>
inline auto await_future<void>(std::shared_ptr<std::future<void>> fut_ptr)
  -> asio::awaitable<void> {
    auto exec = co_await asio::this_coro::executor;
    asio::steady_timer timer(exec);

    while (fut_ptr->wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        timer.expires_after(std::chrono::microseconds(100));
        co_await timer.async_wait(asio::use_awaitable);
    }

    fut_ptr->get();  // 可能抛出异常
}

/**
 * @brief 在指定 executor 上异步执行函数，允许异常穿透（保留原始异常类型）
 *
 * 此函数不会将异常转换为 error_code，而是通过 std::exception_ptr 在协程中重新抛出原始异常。
 * 适用于需要精确捕获特定异常类型的场景。
 *
 * ## 使用示例
 * @code
 *   // 正常情况
 *   try {
 *       int result = co_await co_run(pool.executor(), []() -> int { return 42; });
 *       HKU_INFO("Result: {}", result);
 *   } catch (const std::exception& e) {
 *       HKU_ERROR("Error: {}", e.what());
 *   }
 *
 *   // 异常情况 - 可以捕获原始异常类型
 *   try {
 *       int result = co_await co_run(pool.executor(), []() -> int {
 *           throw std::runtime_error("Specific error");
 *           return 0;
 *       });
 *   } catch (const std::runtime_error& e) {
 *       // 可以直接捕获 std::runtime_error
 *       HKU_ERROR("Runtime error: {}", e.what());
 *   } catch (const std::logic_error& e) {
 *       HKU_ERROR("Logic error: {}", e.what());
 *   }
 * @endcode
 *
 * @param exec 执行器
 * @param func 要执行的函数
 * @return asio::awaitable<T> 异步操作的结果（可能抛出原始异常类型）
 *
 * @see co_run_ec - 将异常转换为 error_code 的版本，适用于统一错误处理
 */
template <typename Executor, typename Func>
auto co_run(Executor exec, Func&& func) -> asio::awaitable<typename std::invoke_result_t<Func>> {
    using ResultType = typename std::invoke_result_t<Func>;

    if constexpr (std::is_void_v<ResultType>) {
        // void 返回类型：completion signature 为 void(std::exception_ptr)
        return asio::async_initiate<decltype(asio::use_awaitable), void(std::exception_ptr)>(
          [exec, func = std::forward<Func>(func)](auto handler) mutable {
              auto io_exec = asio::get_associated_executor(handler);

              exec.execute(
                [func = std::move(func), handler = std::move(handler), io_exec]() mutable {
                    std::exception_ptr e_ptr = nullptr;
                    try {
                        func();
                    } catch (...) {
                        e_ptr = std::current_exception();
                    }

                    asio::post(io_exec,
                               [handler = std::move(handler), e_ptr = std::move(e_ptr)]() mutable {
                                   // Asio 会自动处理 exception_ptr，在 co_await 点抛出
                                   handler(e_ptr);
                               });
                });
          },
          asio::use_awaitable);
    } else {
        // 非 void 返回类型：completion signature 必须包含异常场景
        // 正确签名：void(std::exception_ptr, ResultType)
        return asio::async_initiate<decltype(asio::use_awaitable),
                                    void(std::exception_ptr,
                                         ResultType)  // 关键修复：增加 exception_ptr
                                    >(
          [exec, func = std::forward<Func>(func)](auto handler) mutable {
              auto io_exec = asio::get_associated_executor(handler);

              exec.execute(
                [func = std::move(func), handler = std::move(handler), io_exec]() mutable {
                    std::exception_ptr e_ptr = nullptr;
                    ResultType result{};

                    try {
                        result = func();
                    } catch (...) {
                        e_ptr = std::current_exception();
                    }

                    asio::post(io_exec, [handler = std::move(handler), e_ptr = std::move(e_ptr),
                                         result = std::move(result)]() mutable {
                        // 关键修复：通过 handler 传递异常/结果，而非直接抛出
                        handler(e_ptr, std::move(result));
                    });
                });
          },
          asio::use_awaitable);
    }
}

/**
 * @brief 在指定 executor 上异步执行函数，异常会转换为 boost::system::error_code
 *
 * 此函数将异常转换为 error_code 传递错误状态，适用于不希望异常中断协程执行的场景。
 * 当发生异常时，Boost.Asio 框架会自动将非空的 error_code 转换为 boost::system::system_error 抛出。
 *
 * ## 使用示例
 * @code
 *   // 正常情况
 *   try {
 *       int result = co_await co_run_ec(pool.executor(), []() -> int { return 42; });
 *       HKU_INFO("Result: {}", result);
 *   } catch (const std::exception& e) {
 *       HKU_ERROR("Error: {}", e.what());
 *   }
 *
 *   // 异常情况 - 会被转换为 system_error
 *   try {
 *       int result = co_await co_run_ec(pool.executor(), []() -> int {
 *           throw std::runtime_error("Error");
 *           return 0;
 *       });
 *   } catch (const boost::system::system_error& e) {
 *       HKU_ERROR("Error code: {}", e.code().message());
 *   }
 * @endcode
 *
 * @param exec 执行器
 * @param func 要执行的函数
 * @return boost::asio::awaitable<T> 异步操作的结果（错误时抛出 boost::system::system_error）
 *
 * @see co_run - 允许异常穿透的标准版本，保留原始异常类型
 */
template <typename Executor, typename Func>
auto co_run_ec(Executor exec, Func&& func) -> asio::awaitable<typename std::invoke_result_t<Func>> {
    using ResultType = typename std::invoke_result_t<Func>;

    if constexpr (std::is_void_v<ResultType>) {
        // void 返回类型的特化版本
        return asio::async_initiate<decltype(asio::use_awaitable), void(boost::system::error_code)>(
          [exec, func = std::forward<Func>(func)](auto&& handler) mutable {
              auto io_exec = asio::get_associated_executor(handler);

              exec.execute([func = std::move(func),
                            handler = std::forward<decltype(handler)>(handler), io_exec]() mutable {
                  boost::system::error_code ec;

                  try {
                      func();
                  } catch (const std::exception& e) {
                      ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
                  } catch (...) {
                      ec =
                        boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
                  }

                  boost::asio::post(io_exec,
                                    [handler = std::move(handler), ec]() mutable { handler(ec); });
              });
          },
          boost::asio::use_awaitable);
    } else {
        // 非 void 返回类型的普通版本
        return boost::asio::async_initiate<decltype(boost::asio::use_awaitable),
                                           void(boost::system::error_code, ResultType)>(
          [exec, func = std::forward<Func>(func)](auto&& handler) mutable {
              auto io_exec = boost::asio::get_associated_executor(handler);

              exec.execute([func = std::move(func),
                            handler = std::forward<decltype(handler)>(handler), io_exec]() mutable {
                  ResultType result{};
                  boost::system::error_code ec;

                  try {
                      result = func();
                  } catch (const std::exception& e) {
                      ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
                  } catch (...) {
                      ec =
                        boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
                  }

                  boost::asio::post(io_exec, [handler = std::move(handler), ec,
                                              result = std::move(result)]() mutable {
                      handler(ec, std::move(result));
                  });
              });
          },
          boost::asio::use_awaitable);
    }
}

#endif  // CPP_STANDARD >= CPP_STANDARD_20

}  // namespace hku