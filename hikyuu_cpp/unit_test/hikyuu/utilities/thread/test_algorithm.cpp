/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-25
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/utilities/thread/thread.h>
#include <hikyuu/utilities/cppdef.h>
#include <hikyuu/utilities/thread/ThreadPool.h>
#include <hikyuu/utilities/SpendTimer.h>
#include <hikyuu/utilities/thread/algorithm.h>
#include <thread>

using namespace hku;

#if CPP_STANDARD >= CPP_STANDARD_20

TEST_CASE("test_co_dispatch_basic") {
    ThreadPool pool(4);

    // 测试有返回值的情况
    auto task = co_dispatch(pool.executor(), []() -> int { return 42; });

    CHECK_EQ(task.wait(), 42);
}

TEST_CASE("test_co_dispatch_void") {
    ThreadPool pool(4);

    std::atomic<bool> executed{false};

    auto task = co_dispatch(pool.executor(),
                            [&executed]() { executed.store(true, std::memory_order_relaxed); });

    task.wait();  // 等待完成
    CHECK_UNARY(executed.load());
}

TEST_CASE("test_co_dispatch_delayed_wait") {
    ThreadPool pool(4);

    std::atomic<bool> started{false};

    auto task = co_dispatch(pool.executor(), [&started]() -> int {
        started.store(true, std::memory_order_release);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 42;
    });

    // 等待任务开始执行
    while (!started.load(std::memory_order_acquire)) {
        std::this_thread::yield();
    }

    // 做其他事情...
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    // 需要结果时再等待
    CHECK_EQ(task.wait(), 42);
}

TEST_CASE("test_co_dispatch_fire_and_forget") {
    ThreadPool pool(4);

    std::atomic<int> counter{0};

    // fire-and-forget: 不等待结果，直接销毁 task
    for (int i = 0; i < 10; ++i) {
        co_dispatch(pool.executor(),
                    [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });
        // task 被立即销毁，不等待任务完成
    }

    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    CHECK_EQ(counter.load(), 10);
}

TEST_CASE("test_co_dispatch_exception") {
    ThreadPool pool(4);

    auto task =
      co_dispatch(pool.executor(), []() -> int { throw std::runtime_error("Test exception"); });

    CHECK_THROWS_AS(task.wait(), std::runtime_error);
}

TEST_CASE("test_co_dispatch_lambda_capture") {
    ThreadPool pool(4);

    int value1 = 100;
    std::string value2 = "hello";

    auto task = co_dispatch(pool.executor(),
                            [value1, value2 = std::move(value2)]() -> std::pair<int, std::string> {
                                return {value1, value2};
                            });

    auto result = task.wait();
    CHECK_EQ(result.first, 100);
    CHECK_EQ(result.second, "hello");
}

TEST_CASE("test_co_dispatch_string_return") {
    ThreadPool pool(4);

    std::string expected = "Hello, co_dispatch!";

    auto task = co_dispatch(pool.executor(), [expected]() -> std::string { return expected; });

    CHECK_EQ(task.wait(), expected);
}

TEST_CASE("test_co_dispatch_complex_type") {
    ThreadPool pool(4);

    struct TestData {
        int value;
        std::string text;
        double number;
    };

    auto task =
      co_dispatch(pool.executor(), []() -> TestData { return TestData{42, "test", 3.14}; });

    auto result = task.wait();
    CHECK_EQ(result.value, 42);
    CHECK_EQ(result.text, "test");
    CHECK_EQ(result.number, 3.14);
}

TEST_CASE("test_co_dispatch_move_only_type") {
    ThreadPool pool(4);

    auto task = co_dispatch(pool.executor(),
                            []() -> std::unique_ptr<int> { return std::make_unique<int>(100); });

    auto result = task.wait();
    CHECK_NE(result, nullptr);
    CHECK_EQ(*result, 100);
}

TEST_CASE("test_co_dispatch_multiple_executors") {
    ThreadPool pool1(2);
    ThreadPool pool2(2);

    std::atomic<int> counter{0};

    auto t1 = co_dispatch(pool1.executor(),
                          [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });

    auto t2 = co_dispatch(pool2.executor(),
                          [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });

    t1.wait();
    t2.wait();

    CHECK_EQ(counter.load(), 2);
}

TEST_CASE("test_co_dispatch_stress_test") {
    ThreadPool pool(8);

    const int num_tasks = 100;
    std::vector<dispatch_task<int>> tasks;

    for (int i = 0; i < num_tasks; ++i) {
        tasks.push_back(co_dispatch(pool.executor(), [i]() -> int { return i * 2; }));
    }

    for (int i = 0; i < num_tasks; ++i) {
        CHECK_EQ(tasks[i].wait(), i * 2);
    }
}

TEST_CASE("test_co_dispatch_no_wait_basic") {
    ThreadPool pool(4);

    std::atomic<int> counter{0};

    // 提交任务但不等待
    co_dispatch_no_wait(pool.executor(),
                        [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });

    // 等待任务完成
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    CHECK_EQ(counter.load(), 1);
}

TEST_CASE("test_co_dispatch_no_wait_void") {
    ThreadPool pool(4);

    std::atomic<bool> executed{false};

    co_dispatch_no_wait(pool.executor(),
                        [&executed]() { executed.store(true, std::memory_order_relaxed); });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK_UNARY(executed.load());
}

TEST_CASE("test_co_dispatch_no_wait_with_exception") {
    ThreadPool pool(4);

    // 异常应该被忽略，不会导致程序崩溃
    co_dispatch_no_wait(pool.executor(), []() { throw std::runtime_error("Ignored exception"); });

    // 等待一下确保任务已执行
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 如果能到达这里，说明异常被正确忽略
    CHECK(true);
}

TEST_CASE("test_co_dispatch_no_wait_lambda_capture") {
    ThreadPool pool(4);

    int value1 = 100;
    std::string value2 = "hello";
    std::atomic<bool> executed{false};

    co_dispatch_no_wait(pool.executor(), [value1, value2 = std::move(value2), &executed]() {
        CHECK_EQ(value1, 100);
        CHECK_EQ(value2, "hello");
        executed.store(true, std::memory_order_relaxed);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK_UNARY(executed.load());
}

TEST_CASE("test_co_dispatch_no_wait_performance") {
    ThreadPool pool(8);

    const int num_tasks = 1000;
    std::atomic<int> counter{0};

    // 测试 fire-and-forget 模式的性能
    for (int i = 0; i < num_tasks; ++i) {
        co_dispatch_no_wait(pool.executor(),
                            [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });
    }

    // 等待所有任务完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    CHECK_EQ(counter.load(), num_tasks);
}

TEST_CASE("test_co_dispatch_no_wait_multiple_executors") {
    ThreadPool pool1(2);
    ThreadPool pool2(2);

    std::atomic<int> counter{0};

    co_dispatch_no_wait(pool1.executor(),
                        [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });

    co_dispatch_no_wait(pool2.executor(),
                        [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    CHECK_EQ(counter.load(), 2);
}

// ============================================================================
// 协程中的使用示例测试
// ============================================================================

/**
 * @brief 辅助协程函数：测试 await_future 在协程中的基本使用
 */
static asio::awaitable<void> test_await_future_in_coro_helper() {
    ThreadPool pool(4);

    // 提交任务到线程池，获取 std::future
    std::future<int> fut = pool.submit([]() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 42;
    });

    // 在协程中异步等待 future
    int result = co_await await_future(std::move(fut));
    CHECK_EQ(result, 42);
}

TEST_CASE("test_await_future_in_coroutine") {
    asio::io_context ctx;
    asio::co_spawn(ctx, test_await_future_in_coro_helper(), asio::detached);
    ctx.run();
}

/**
 * @brief 辅助协程函数：测试 co_dispatch 在协程中使用 await_future 等待
 */
static asio::awaitable<void> test_co_dispatch_in_coro_helper() {
    ThreadPool pool(4);

    // 提交任务并获取 dispatch_task
    auto task = co_dispatch(pool.executor(), []() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 100;
    });

    // 在协程中通过 await_future 异步等待（推荐方式）
    int result = co_await await_future(std::move(task.fut));
    CHECK_EQ(result, 100);
}

TEST_CASE("test_co_dispatch_in_coroutine") {
    asio::io_context ctx;
    asio::co_spawn(ctx, test_co_dispatch_in_coro_helper(), asio::detached);
    ctx.run();
}

/**
 * @brief 辅助协程函数：测试混合使用异步模式
 */
static asio::awaitable<void> test_mixed_async_in_coro_helper() {
    ThreadPool pool(4);

    // 1. 使用 co_dispatch 提交需要等待的任务
    auto task = co_dispatch(pool.executor(), []() -> int { return 10; });

    // 2. 使用 co_dispatch_no_wait 提交后台任务
    std::atomic<bool> background_done{false};
    co_dispatch_no_wait(pool.executor(), [&background_done]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        background_done.store(true, std::memory_order_relaxed);
    });

    // 3. 在协程中通过 await_future 等待 co_dispatch 的结果
    int result = co_await await_future(std::move(task.fut));
    CHECK_EQ(result, 10);

    // 4. 等待后台任务完成（使用 timer 轮询）
    auto exec = co_await asio::this_coro::executor;
    asio::steady_timer timer(exec);
    while (!background_done.load()) {
        timer.expires_after(std::chrono::microseconds(100));
        co_await timer.async_wait(asio::use_awaitable);
    }
    CHECK_UNARY(background_done.load());
}

TEST_CASE("test_mixed_async_patterns_in_coroutine") {
    asio::io_context ctx;
    asio::co_spawn(ctx, test_mixed_async_in_coro_helper(), asio::detached);
    ctx.run();
}

/**
 * @brief 辅助协程函数：测试并发执行多个异步任务
 */
static asio::awaitable<void> test_concurrent_tasks_in_coro_helper() {
    ThreadPool pool(8);

    // 并发提交多个任务
    std::vector<dispatch_task<int>> tasks;
    for (int i = 0; i < 5; ++i) {
        tasks.push_back(co_dispatch(pool.executor(), [i]() -> int {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return i * i;
        }));
    }

    // 并发等待所有结果 - 使用 await_future
    std::vector<int> results;
    for (auto& task : tasks) {
        results.push_back(co_await await_future(std::move(task.fut)));
    }

    // 验证结果
    CHECK_EQ(results.size(), 5u);
    for (int i = 0; i < 5; ++i) {
        CHECK_EQ(results[i], i * i);
    }
}

TEST_CASE("test_concurrent_dispatch_tasks_in_coroutine") {
    asio::io_context ctx;
    asio::co_spawn(ctx, test_concurrent_tasks_in_coro_helper(), asio::detached);
    ctx.run();
}

#endif  // CPP_STANDARD >= CPP_STANDARD_20

/**
 * @defgroup test_hikyuu_algorithm test_hikyuu_algorithm
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_parallelIndexRange") {
    /** @arg start=0, end=0 */
    std::vector<range_t> ranges = parallelIndexRange(0, 0);
    CHECK_UNARY(ranges.empty());

    /** @arg start=2, end=2 */
    ranges = parallelIndexRange(2, 2);
    CHECK_UNARY(ranges.empty());

    /** @arg start=1, end=0 */
    ranges = parallelIndexRange(1, 0);
    CHECK_UNARY(ranges.empty());

    /** @arg start=0, end=3 */
    ranges = parallelIndexRange(0, 3);
    CHECK_UNARY(ranges.size() > 0);

    std::vector<size_t> expect(3);
    for (size_t i = 0; i < expect.size(); ++i) {
        expect[i] = i;
    }

    std::vector<size_t> result;
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg start=0, end=100 */
    ranges = parallelIndexRange(0, 100);
    CHECK_UNARY(ranges.size() > 0);

    expect.resize(100);
    for (size_t i = 0; i < expect.size(); ++i) {
        expect[i] = i;
    }

    result.clear();
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg start=1, end=100 */
    ranges = parallelIndexRange(1, 100);
    CHECK_UNARY(ranges.size() > 0);

    expect.resize(99);
    for (size_t i = 0; i < expect.size(); ++i) {
        expect[i] = i + 1;
    }

    result.clear();
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg start=99, end=100 */
    ranges = parallelIndexRange(99, 100);
    CHECK_UNARY(ranges.size() > 0);

    expect.resize(1);
    expect[0] = 99;

    result.clear();
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }

    /** @arg start=99, end=100 */
    ranges = parallelIndexRange(17, 50);
    CHECK_UNARY(ranges.size() > 0);

    expect.resize(50 - 17);
    for (size_t i = 0; i < expect.size(); ++i) {
        expect[i] = i + 17;
    }

    result.clear();
    for (const auto& r : ranges) {
        for (size_t i = r.first; i < r.second; ++i) {
            result.push_back(i);
        }
    }

    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0; i < expect.size(); i++) {
        CHECK_EQ(result[i], expect[i]);
    }
}

TEST_CASE("test_parallelIndexRange2") {
    std::vector<std::pair<size_t, size_t>> expect{{0, 1}, {1, 2}};
    auto result = parallelIndexRange(0, 2);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0, len = expect.size(); i < len; i++) {
        CHECK_EQ(result[i].first, expect[i].first);
        CHECK_EQ(result[i].second, expect[i].second);
    }

    size_t cpu_num = std::thread::hardware_concurrency();
    if (cpu_num == 32) {
        result = parallelIndexRange(0, 100);
        expect = {{0, 3},   {3, 6},   {6, 9},   {9, 12},  {12, 15}, {15, 18}, {18, 21}, {21, 24},
                  {24, 27}, {27, 30}, {30, 33}, {33, 36}, {36, 39}, {39, 42}, {42, 45}, {45, 48},
                  {48, 51}, {51, 54}, {54, 57}, {57, 60}, {60, 63}, {63, 66}, {66, 69}, {69, 72},
                  {72, 75}, {75, 78}, {78, 81}, {81, 84}, {84, 87}, {87, 90}, {90, 93}, {93, 96},
                  {96, 97}, {97, 98}, {98, 99}, {99, 100}};
        CHECK_EQ(result.size(), expect.size());
        for (size_t i = 0, len = expect.size(); i < len; i++) {
            CHECK_EQ(result[i].first, expect[i].first);
            CHECK_EQ(result[i].second, expect[i].second);
        }

    } else if (cpu_num == 8) {
        result = parallelIndexRange(0, 35);
        expect = {{0, 4},   {4, 8},   {8, 12},  {12, 16}, {16, 20}, {20, 24},
                  {24, 28}, {28, 32}, {32, 33}, {33, 34}, {34, 35}};
        CHECK_EQ(result.size(), expect.size());
        for (size_t i = 0, len = expect.size(); i < len; i++) {
            CHECK_EQ(result[i].first, expect[i].first);
            CHECK_EQ(result[i].second, expect[i].second);
        }
    }
}

TEST_CASE("test_parallel_for_index") {
    std::vector<size_t> values(100);
    for (size_t i = 0, len = values.size(); i < len; i++) {
        values[i] = i;
    }

    auto result = parallel_for_index(0, values.size(), [](size_t i) { return i + 1; });

    std::vector<size_t> expect(100);
    for (size_t i = 0, len = expect.size(); i < len; i++) {
        expect[i] = i + 1;
    }
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0, len = expect.size(); i < len; i++) {
        CHECK_EQ(result[i], expect[i]);
    }
}

TEST_CASE("test_parallel_for_index_single") {
    std::vector<size_t> values(100);
    for (size_t i = 0, len = values.size(); i < len; i++) {
        values[i] = i;
    }

    auto result = parallel_for_index_single(0, values.size(), [](size_t i) { return i + 1; });
    // auto result = parallel_for_index_single<std::function<size_t(size_t)>, MQStealThreadPool>(
    //   0, values.size(), [](size_t i) { return i + 1; });

    std::vector<size_t> expect(100);
    for (size_t i = 0, len = expect.size(); i < len; i++) {
        expect[i] = i + 1;
    }
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = 0, len = expect.size(); i < len; i++) {
        CHECK_EQ(result[i], expect[i]);
    }
}

/** @} */
