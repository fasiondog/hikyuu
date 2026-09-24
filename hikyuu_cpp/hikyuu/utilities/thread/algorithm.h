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
#include <memory>
#include "ThreadPool.h"
#include "MQThreadPool.h"
#include "StealThreadPool.h"
#include "MQStealThreadPool.h"
#include "GlobalStealThreadPool.h"

#if CPP_STANDARD >= CPP_STANDARD_20
#include "hikyuu/utilities/net.h"
#include <type_traits>
#include <exception>
#endif

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

//----------------------------------------------------------------
// Note: apart from ThreadPool/MQThreadPool, the other thread pools use
//       thread_local, which is essentially a global variable, so they are only suitable to be used
//       as a global singleton, otherwise different thread pool instances would affect each other
//       and cause errors. An independent thread pool is created for the calculation every time. If
//       they are all pure calculations (with little IO), it is recommended to create the global
//       thread pool and use it for the calculation.
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
// Create the global task stealing thread pool; its main purpose is the parallelization of the
// computation intensive or slightly mixed IO work, it is not suitable for the pure IO
// parallelization The parallel_for series above creates an independent thread pool every time.
// note: it is global within the program, initializing it once is enough and a repeated
// initialization is ignored
//----------------------------------------------------------------
extern HKU_UTILS_API std::unique_ptr<GlobalStealThreadPool> global_steal_thread_pool;

void HKU_UTILS_API init_global_task_group(size_t work_num = 0);

void HKU_UTILS_API release_global_task_group();

inline GlobalStealThreadPool* get_global_task_group() {
    return global_steal_thread_pool.get();
}

size_t HKU_UTILS_API get_global_task_group_work_num();

template <typename FutureContainer>
void wait_for_all_non_blocking(GlobalStealThreadPool& pool, FutureContainer& futures) {
    // If the current thread is a worker thread, its child tasks are added to the front of its own
    // queue and the other threads cannot get the child tasks, so a wake-up is needed
    // When it is not a worker thread, its child tasks are added to the master queue and no active
    // wake-up is needed
    bool is_work_thread = GlobalStealThreadPool::is_work_thread();
    if (is_work_thread) {
        pool.wake_up();
    }

    bool all_ready = false;
    auto init_delay = std::chrono::microseconds(1);
    auto delay = init_delay;
    const auto max_delay = std::chrono::microseconds(1000);
    int spin_count = 0;

    while (!all_ready && !pool.done()) {
        all_ready = true;
        for (auto& future : futures) {
            if (future.wait_for(std::chrono::nanoseconds(0)) != std::future_status::ready) {
                all_ready = false;
                break;
            }
        }

        // If not all the tasks are finished, try to execute the accumulated tasks
        if (!all_ready) {
            if (pool.run_available_task_once()) {
                delay = init_delay;
                spin_count = 0;
            } else if (pool.done()) {
                return;
            } else {
                // When there is no task to steal at present, the unfinished tasks may be running in
                // the other worker threads. First yield the time slice several times, then enter
                // the microsecond level exponential backoff (with an upper limit of 1ms), ensuring
                // that there is no leftover task when the subsequent get of the caller throws an
                // exception, while avoiding a long busy wait.
                if (spin_count < 8) {
                    ++spin_count;
                    std::this_thread::yield();
                } else {
                    std::this_thread::sleep_for(delay);
                    if (delay < max_delay) {
                        delay = std::min(delay * 2, max_delay);
                    }
                }
            }
        }
    }
}

/** A task submitted with global_submit_task must be used together with global_wait_task and
 *  global_wake_up */
template <typename FunctionType>
auto global_submit_task(FunctionType&& f) {
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

        // If the task is not finished, try to execute the local tasks
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
auto global_parallel_for_index_void(size_t start, size_t end, FunctionType&& f,
                                    size_t threshold = 2, bool enable_nested = true) {
    HKU_IF_RETURN(start >= end, void());

    // If the number of the tasks is less than the threshold, or the current thread is a worker
    // thread and the nesting is forbidden, it is executed directly
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
auto global_parallel_for_index(size_t start, size_t end, FunctionType&& f, size_t threshold = 2,
                               bool enable_nested = true) {
    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    HKU_IF_RETURN(start >= end, ret);

    ret.reserve(end - start);

    // Check whether the current thread is already executing a task; if so it degrades to the serial
    // execution
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
void global_parallel_for_index_void_single(size_t start, size_t end, FunctionType&& f,
                                           size_t threshold = 1, bool enable_nested = true) {
    HKU_IF_RETURN(start >= end, void());

    // Check whether the current thread is already executing a task; if so it degrades to the serial
    // execution
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
auto global_parallel_for_index_single(size_t start, size_t end, FunctionType&& f,
                                      size_t threshold = 1, bool enable_nested = true) {
    std::vector<typename std::invoke_result<FunctionType, size_t>::type> ret;
    HKU_IF_RETURN(start >= end, ret);

    ret.reserve(end - start);

    // Check whether the current thread is already executing a task; if so it degrades to the serial
    // execution
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
// Coroutine
//----------------------------------------------------------------
namespace asio = net::asio;

/**
 * @brief An adapter function for waiting for a std::future in a coroutine
 *
 * This is the recommended usage, used to elegantly wait for a traditional std::future in a
 * boost::asio coroutine.
 *
 * ## Usage scenarios
 *
 * ### 1. Wrapping the existing future based APIs
 * @code
 *   // Assume there is a function returning a std::future
 *   std::future<int> compute_async();
 *
 *   // Use it in a coroutine
 *   asio::awaitable<void> my_coroutine() {
 *       int result = co_await await_future(compute_async());
 *   }
 * @endcode
 *
 * ### 2. Execute a task in the thread pool and wait in a coroutine
 * @code
 *   asio::awaitable<void> coroutine_with_pool() {
 *       ThreadPool pool(4);
 *
 *       // Submit the task to get the future
 *       auto fut = pool.submit([]() { return heavy_compute(); });
 *
 *       // Wait for the result in the coroutine (without blocking the event loop)
 *       int result = co_await await_future(std::move(fut));
 *   }
 * @endcode
 *
 * ### 3. Shared future (multiple coroutines wait for the same task)
 * @code
 *   asio::awaitable<void> shared_future_example() {
 *       ThreadPool pool(4);
 *       auto fut_ptr = std::make_shared<std::future<int>>(pool.submit(task));
 *
 *       // Multiple coroutines can wait for the same task
 *       int r1 = co_await await_future(fut_ptr);
 *       int r2 = co_await await_future(fut_ptr);
 *   }
 * @endcode
 *
 * ## Implementation principle
 *
 * `asio::steady_timer` is used for an efficient polling (with an interval of 100 microseconds),
 * avoiding the busy wait:
 * 1. Periodically check whether the future is ready
 * 2. Suspend the coroutine asynchronously through the timer when it is not ready, releasing the
 *    execution right to the event loop
 * 3. Resume the coroutine immediately and return the result after the future is ready
 *
 * ## Exception handling
 *
 * If the future contains an exception, it is rethrown into the coroutine:
 * @code
 *   try {
 *       auto result = co_await await_future(std::move(fut));
 *   } catch (const std::exception& e) {
 *       // Handle the exception in the future
 *   }
 * @endcode
 *
 * @tparam T the return value type of the future
 * @param fut the std::future<T> object (an rvalue reference)
 * @return asio::awaitable<T> an object that can be co_awaited in a coroutine
 *
 * @see co_run - the standard asynchronous coroutine task execution interface
 * @see co_dispatch_no_wait - the fire-and-forget mode
 */
template <typename T>
auto await_future(std::future<T> fut) -> asio::awaitable<T> {
    auto exec = co_await asio::this_coro::executor;
    asio::steady_timer timer(exec);

    // Use the timer for an efficient polling, avoiding the busy wait
    while (fut.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        timer.expires_after(std::chrono::microseconds(100));
        co_await timer.async_wait(asio::use_awaitable);
    }

    co_return fut.get();  // It may throw an exception
}

/**
 * @brief The void specialization version
 *
 * It is used to wait for a std::future<void> object without a return value.
 * The behavior is the same as the template version, it just does not return a value; it is mainly
 * used to wait for the task completion.
 *
 * @param fut the std::future<void> object
 * @return asio::awaitable<void> an object that can be co_awaited in a coroutine
 *
 * @example
 *   asio::awaitable<void> example() {
 *       ThreadPool pool(4);
 *       auto fut = pool.submit([]() {
 *           // Perform some operations
 *           do_something();
 *       });
 *
 *       // Wait for the task to be finished
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

    fut.get();  // It may throw an exception
}

/**
 * @brief The shared_ptr version of await_future (suitable for the scenarios where the future needs
 * to be shared)
 *
 * It is used when multiple coroutines need to wait for the result of the same asynchronous task.
 * The lifetime of std::future is managed through std::shared_ptr, allowing multiple coroutines to
 * safely wait for the same task.
 *
 * ## Usage scenarios
 *
 * ### 1. Broadcast mode - multiple coroutines wait for the same event
 * @code
 *   asio::awaitable<void> broadcast_example() {
 *       ThreadPool pool(4);
 *       auto fut_ptr = std::make_shared<std::future<int>>(pool.submit([]() {
 *           return compute_expensive_value();
 *       }));
 *
 *       // Start multiple coroutines, all waiting for the same calculation result
 *       co_spawn(co_await asio::this_coro::executor,
 *                [fut_ptr]() -> asio::awaitable<void> {
 *                    int result = co_await await_future(fut_ptr);
 *                    // Use the result...
 *                }, asio::detached);
 *
 *       co_spawn(co_await asio::this_coro::executor,
 *                [fut_ptr]() -> asio::awaitable<void> {
 *                    int result = co_await await_future(fut_ptr);
 *                    // Use the result...
 *                }, asio::detached);
 *   }
 * @endcode
 *
 * ### 2. Cache the asynchronous result
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
 * ## Notes
 *
 * 1. All the coroutines waiting for the same shared_ptr are resumed almost at the same time when
 * the future is ready
 * 2. Exception handling: if the future contains an exception, every waiting coroutine receives the
 *    same exception
 * 3. Performance: compared with passing the future directly, the shared_ptr version has a slight
 *    performance overhead
 *
 * @tparam T the return value type of the future
 * @param fut_ptr the shared future pointer std::shared_ptr<std::future<T>>
 * @return asio::awaitable<T> an object that can be co_awaited in a coroutine
 *
 * @see await_future(std::future<T>) - the direct future version
 * @see co_run - the standard asynchronous coroutine task execution interface
 * @see co_dispatch_no_wait - the fire-and-forget mode
 */
template <typename T>
auto await_future(std::shared_ptr<std::future<T>> fut_ptr) -> asio::awaitable<T> {
    auto exec = co_await asio::this_coro::executor;
    asio::steady_timer timer(exec);

    while (fut_ptr->wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        timer.expires_after(std::chrono::microseconds(100));
        co_await timer.async_wait(asio::use_awaitable);
    }

    co_return fut_ptr->get();  // It may throw an exception
}

/**
 * @brief The void specialization version (shared_ptr)
 *
 * The void specialization of the shared_ptr version of await_future, used to wait for a shared
 * future without a return value. It is mainly used in the scenarios where multiple coroutines need
 * to synchronously wait for an asynchronous operation to be finished.
 *
 * @param fut_ptr the shared void future pointer std::shared_ptr<std::future<void>>
 * @return asio::awaitable<void> an object that can be co_awaited in a coroutine
 *
 * @example
 *   asio::awaitable<void> shared_void_example() {
 *       ThreadPool pool(4);
 *       auto event = std::make_shared<std::future<void>>(
 *           pool.submit([]() {
 *               // The time-consuming initialization
 *               initialize_system();
 *           })
 *       );
 *
 *       // Multiple service coroutines wait for the system initialization to be finished
 *       co_await await_future(event);
 *       co_await await_future(event);  // Another coroutine waits in the same way
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

    fut_ptr->get();  // It may throw an exception
}

/**
 * @brief Execute the function asynchronously on the given executor, allowing the exception to pass
 * through (keeping the original exception type)
 *
 * This function does not convert the exception into an error_code; instead it rethrows the original
 * exception in the coroutine through std::exception_ptr.
 * It is suitable for the scenarios where a specific exception type needs to be captured precisely.
 *
 * ## Usage example
 * @code
 *   // The normal case
 *   try {
 *       int result = co_await co_run(pool.executor(), []() -> int { return 42; });
 *       HKU_INFO("Result: {}", result);
 *   } catch (const std::exception& e) {
 *       HKU_ERROR("Error: {}", e.what());
 *   }
 *
 *   // The abnormal case - the original exception type can be caught
 *   try {
 *       int result = co_await co_run(pool.executor(), []() -> int {
 *           throw std::runtime_error("Specific error");
 *           return 0;
 *       });
 *   } catch (const std::runtime_error& e) {
 *       // std::runtime_error can be caught directly
 *       HKU_ERROR("Runtime error: {}", e.what());
 *   } catch (const std::logic_error& e) {
 *       HKU_ERROR("Logic error: {}", e.what());
 *   }
 * @endcode
 *
 * @param exec executor
 * @param func the function to be executed
 * @return asio::awaitable<T> the result of the asynchronous operation (the original exception type
 * may be thrown)
 *
 * @see co_run_ec - the version converting the exception into an error_code, suitable for a unified
 * error handling
 */
template <typename Executor, typename Func>
auto co_run(Executor exec, Func&& func) -> asio::awaitable<typename std::invoke_result_t<Func>> {
    using ResultType = typename std::invoke_result_t<Func>;

    if constexpr (std::is_void_v<ResultType>) {
        // The void return type: the completion signature is void(std::exception_ptr)
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
                                   // Asio handles the exception_ptr automatically and throws it at
                                   // the co_await point
                                   handler(e_ptr);
                               });
                });
          },
          asio::use_awaitable);
    } else {
        // The non-void return type: the completion signature must contain the exception scenario
        // The correct signature: void(std::exception_ptr, ResultType)
        return asio::async_initiate<decltype(asio::use_awaitable),
                                    void(std::exception_ptr,
                                         ResultType)  // Key fix: add exception_ptr
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
                        // Key fix: pass the exception/result through the handler instead of
                        // throwing directly
                        handler(e_ptr, std::move(result));
                    });
                });
          },
          asio::use_awaitable);
    }
}

/**
 * @brief Execute the function asynchronously on the given executor; the exception is converted into
 * a net::error_code
 *
 * This function converts the exception into an error_code to pass the error state; it is suitable
 * for the scenarios where the exception is not expected to interrupt the coroutine execution.
 * When an exception occurs, the Boost.Asio framework automatically converts a non-empty error_code
 * into a thrown boost::system::system_error.
 *
 * ## Usage example
 * @code
 *   // The normal case
 *   try {
 *       int result = co_await co_run_ec(pool.executor(), []() -> int { return 42; });
 *       HKU_INFO("Result: {}", result);
 *   } catch (const std::exception& e) {
 *       HKU_ERROR("Error: {}", e.what());
 *   }
 *
 *   // The abnormal case - it is converted into a system_error
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
 * @param exec executor
 * @param func the function to be executed
 * @return net::awaitable<T> the result of the asynchronous operation (boost::system::system_error
 * is thrown on an error)
 *
 * @see co_run - the standard version allowing the exception to pass through, keeping the original
 * exception type
 */
template <typename Executor, typename Func>
auto co_run_ec(Executor exec, Func&& func) -> asio::awaitable<typename std::invoke_result_t<Func>> {
    using ResultType = typename std::invoke_result_t<Func>;

    if constexpr (std::is_void_v<ResultType>) {
        // The specialization version for the void return type
        return asio::async_initiate<decltype(asio::use_awaitable), void(net::error_code)>(
          [exec, func = std::forward<Func>(func)](auto&& handler) mutable {
              auto io_exec = asio::get_associated_executor(handler);

              exec.execute([func = std::move(func),
                            handler = std::forward<decltype(handler)>(handler), io_exec]() mutable {
                  net::error_code ec;

                  try {
                      func();
                  } catch (const std::exception&) {
                      ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
                  } catch (...) {
                      ec =
                        boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
                  }

                  asio::post(io_exec,
                             [handler = std::move(handler), ec]() mutable { handler(ec); });
              });
          },
          asio::use_awaitable);
    } else {
        // The ordinary version for the non-void return type
        return asio::async_initiate<decltype(asio::use_awaitable),
                                    void(net::error_code, ResultType)>(
          [exec, func = std::forward<Func>(func)](auto&& handler) mutable {
              auto io_exec = asio::get_associated_executor(handler);

              exec.execute([func = std::move(func),
                            handler = std::forward<decltype(handler)>(handler), io_exec]() mutable {
                  ResultType result{};
                  net::error_code ec;

                  try {
                      result = func();
                  } catch (const std::exception&) {
                      ec = boost::system::errc::make_error_code(boost::system::errc::io_error);
                  } catch (...) {
                      ec =
                        boost::system::errc::make_error_code(boost::system::errc::invalid_argument);
                  }

                  asio::post(io_exec, [handler = std::move(handler), ec,
                                       result = std::move(result)]() mutable {
                      handler(ec, std::move(result));
                  });
              });
          },
          asio::use_awaitable);
    }
}

#endif  // CPP_STANDARD >= CPP_STANDARD_20

}  // namespace hku