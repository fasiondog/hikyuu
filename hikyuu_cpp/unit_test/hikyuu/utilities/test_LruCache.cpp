/*
 * test_LruCache.cpp
 *
 *  Created on: 2026-01-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/utilities/LruCache.h>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>

using namespace hku;

/**
 * @defgroup test_hikyuu_LruCache test_hikyuu_LruCache
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par 检测点: 基本功能测试 */
TEST_CASE("test_LruCache_basic") {
    LruCache<int, std::string> cache(3, 0);  // 容量为3的缓存，溢出容量为0以保持原有行为

    /** @arg 测试插入和获取 */
    cache.insert(1, "one");
    cache.insert(2, "two");
    cache.insert(3, "three");

    CHECK_EQ(cache.get(1), "one");
    CHECK_EQ(cache.get(2), "two");
    CHECK_EQ(cache.get(3), "three");

    /** @arg 测试LRU淘汰机制 */
    cache.insert(4, "four");  // 此时链表状态 4,1,3

    CHECK_EQ(cache.get(1), "one");    // 非严格LRU模式，仍旧存在
    CHECK_EQ(cache.get(2), "");       // 非严格LRU模式，已淘汰
    CHECK_EQ(cache.get(3), "three");  // 仍存在
    CHECK_EQ(cache.get(4), "four");   // 最新插入

    /** @arg 测试访问更新LRU顺序 */
    cache.get(4);             // 访问4,标记为已读取
    cache.insert(5, "five");  // 此时链表状态 3,5,4

    CHECK_EQ(cache.get(1), "");       // 被淘汰
    CHECK_EQ(cache.get(3), "three");  // 仍存在
    CHECK_EQ(cache.get(4), "four");   // 仍存在
    CHECK_EQ(cache.get(5), "five");   // 新插入

    /** @arg 测试容量和大小 */
    CHECK_EQ(cache.capacity(), 3);
    CHECK_EQ(cache.size(), 3);
    CHECK_UNARY(!cache.empty());

    /** @arg 测试更新已存在的键 */
    cache.insert(1, "two_updated");
    CHECK_EQ(cache.get(1), "two_updated");
}

/** @par 检测点: 移动语义功能测试 */
TEST_CASE("test_LruCache_move_semantics") {
    LruCache<int, std::string> cache(3, 0);  // 溢出容量为0以保持原有行为

    /** @arg 测试移动语义insert */
    std::string value = "test_value";
    std::string original_value = value;  // 保存原始值

    cache.insert(1, std::move(value));  // 使用移动语义插入

    CHECK_EQ(cache.get(1), original_value);  // 确保值正确存储
    // value变量现在可能为空，因为我们移动了它

    /** @arg 测试移动语义更新现有键 */
    std::string new_value = "new_test_value";
    std::string original_new_value = new_value;  // 保存原始值

    cache.insert(1, std::move(new_value));       // 使用移动语义更新已存在的键
    CHECK_EQ(cache.get(1), original_new_value);  // 确保值被正确更新

    /** @arg 测试移动语义与LRU淘汰机制 */
    std::string value2 = "value2";
    std::string original_value2 = value2;
    std::string value3 = "value3";
    std::string original_value3 = value3;
    std::string value4 = "value4";
    std::string original_value4 = value4;

    cache.insert(2, std::move(value2));  // 插入第二个值
    cache.insert(3, std::move(value3));  // 插入第三个值
    CHECK_EQ(cache.get(2), original_value2);
    CHECK_EQ(cache.get(3), original_value3);

    cache.insert(4, std::move(value4));  // 此时应该触发LRU淘汰，键1可能被移除
    CHECK_EQ(cache.get(4), original_value4);

    // 验证哪些键还存在
    // 由于LRU机制，最久未访问的键(可能是2或3之前的1)会被移除
    CHECK_EQ(cache.size(), 3);  // 确保缓存大小正确
}

/** @par 检测点: tryGet功能测试 */
TEST_CASE("test_LruCache_tryGet") {
    LruCache<int, std::string> cache(3, 0);  // 溢出容量为0以保持原有行为

    /** @arg 测试tryGet在键不存在时的行为 */
    std::string value;
    bool found = cache.tryGet(1, value);
    CHECK_UNARY(!found);             // 键不存在，应返回false
    CHECK_EQ(value, std::string{});  // 值应为默认构造值

    /** @arg 测试tryGet在键存在时的行为 */
    cache.insert(1, "one");
    bool found2 = cache.tryGet(1, value);
    CHECK_UNARY(found2);     // 键存在，应返回true
    CHECK_EQ(value, "one");  // 值应正确返回

    /** @arg 测试tryGet更新LRU顺序 */
    cache.insert(2, "two");
    cache.insert(3, "three");
    CHECK_EQ(cache.size(), 3);  // 确保三个元素都在

    std::string value3;
    bool found3 = cache.tryGet(2, value3);  // 访问2，使其变为最新
    CHECK_UNARY(found3);
    CHECK_EQ(value3, "two");

    cache.insert(4, "four");  // 此时应该淘汰最早访问的元素，由于2被访问了，所以1应该被淘汰
    std::string value4;
    bool found4 = cache.tryGet(1, value4);
    CHECK_UNARY(!found4);             // 1应该已被淘汰
    CHECK_EQ(cache.get(2), "two");    // 2应该还在，因为刚访问过
    CHECK_EQ(cache.get(3), "three");  // 3应该还在
    CHECK_EQ(cache.get(4), "four");   // 4是最新插入的
}

/** @par 检测点: 边界情况测试 */
TEST_CASE("test_LruCache_edge_cases") {
    LruCache<int, int> cache(0, 0);  // 容量为0的缓存，溢出容量也为0

    /** @arg 测试容量为0的情况（无限制容量）*/
    cache.insert(1, 100);
    CHECK_EQ(cache.get(1), 100);  // 应该返回存储的值，因为容量为0表示无限制
    CHECK_UNARY(!cache.empty());  // 缓存不应该为空，因为有元素

    /** @arg 测试容量调整 */
    cache.resize(2);
    cache.insert(1, 100);
    cache.insert(2, 200);
    CHECK_EQ(cache.size(), 2);
    CHECK_EQ(cache.get(1), 100);
    CHECK_EQ(cache.get(2), 200);

    /** @arg 测试清空 */
    cache.clear();
    CHECK_UNARY(cache.empty());
    CHECK_EQ(cache.size(), 0);
}

/** @par 检测点: contains和remove功能 */
TEST_CASE("test_LruCache_contains_remove") {
    LruCache<std::string, int> cache(3, 0);  // 溢出容量为0以保持原有行为

    /** @arg 测试contains功能 */
    cache.insert("key1", 100);
    cache.insert("key2", 200);
    CHECK_UNARY(cache.contains("key1"));
    CHECK_UNARY(cache.contains("key2"));
    CHECK_UNARY(!cache.contains("key3"));

    /** @arg 测试remove功能 */
    bool removed = cache.remove("key1");
    CHECK_UNARY(removed);
    CHECK_UNARY(!cache.contains("key1"));
    CHECK_EQ(cache.get("key1"), 0);

    /** @arg 测试删除不存在的键 */
    removed = cache.remove("nonexistent");
    CHECK_UNARY(!removed);

    /** @arg 测试remove后的新插入 */
    cache.insert("key3", 300);
    cache.insert("key4", 400);
    CHECK_UNARY(cache.contains("key3"));
    CHECK_UNARY(cache.contains("key4"));
    CHECK_UNARY(cache.contains("key2"));  // key2应该还在，因为key1已经被删除了
}

/** @par 检测点: 线程安全测试 */
TEST_CASE("test_LruCache_thread_safety") {
    const int num_threads = 10;
    const int ops_per_thread = 100;
    std::vector<std::thread> threads;

    // 使用智能指针管理cache
    auto cache = std::make_shared<LruCache<int, int, std::shared_mutex>>(
      1000, 0);  // 增加容量以避免并发测试时的LRU淘汰，溢出容量为0

    // 并发写入测试
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([=, cache_ptr = cache]() {  // 每个线程持有cache的shared_ptr
            for (int i = 0; i < ops_per_thread; ++i) {
                int key = t * ops_per_thread + i;
                cache_ptr->insert(key, key * 2);
            }
        });
    }

    for (auto& th : threads) {
        th.join();
    }

    // 验证写入的结果
    for (int t = 0; t < num_threads; ++t) {
        for (int i = 0; i < ops_per_thread; ++i) {
            int key = t * ops_per_thread + i;
            int expected_value = key * 2;
            CHECK_EQ(cache->get(key), expected_value);
        }
    }

    // 并发读写测试 - 使用独立的作用域确保这些线程完全结束
    {
        std::atomic<bool> stop_flag(false);
        std::vector<std::thread> rw_threads;

        // 为了并发读写测试，使用更大的容量避免LRU影响
        cache->setOverflow(0);  // 设置溢出容量为0
        cache->resize(2000);    // 调整容量以避免LRU影响

        // 写线程
        for (int w = 0; w < 3; ++w) {
            rw_threads.emplace_back(
              [w, cache_ptr = cache, &stop_flag]() {  // 每个线程持有cache的shared_ptr
                  int key_offset = w * 1000;
                  int key = key_offset;
                  while (!stop_flag.load()) {
                      cache_ptr->insert(key % 500, key * 3);  // 在小范围内循环使用键
                      key++;
                      std::this_thread::sleep_for(std::chrono::microseconds(100));
                  }
              });
        }

        // 读线程
        for (int r = 0; r < 3; ++r) {
            rw_threads.emplace_back(
              [r, cache_ptr = cache, &stop_flag]() {  // 每个线程持有cache的shared_ptr
                  int key_offset = r * 1000;
                  int key = key_offset;
                  while (!stop_flag.load()) {
                      int val = cache_ptr->get(key % 500);
                      // 不再验证特定值，因为并发写入可能导致值的变化
                      key++;
                      std::this_thread::sleep_for(std::chrono::microseconds(100));
                  }
              });
        }

        // 运行一段时间后停止
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        stop_flag.store(true);

        for (auto& th : rw_threads) {
            th.join();
        }

        // rw_threads在这里离开作用域并被销毁
    }

    // cache会在所有线程都结束后才离开作用域，确保安全
}

/** @par 检测点: 综合功能测试 */
TEST_CASE("test_LruCache_comprehensive") {
    LruCache<int, std::string> cache(3, 0);  // 容量为3的缓存，溢出容量为0

    /** @arg 测试初始状态 */
    CHECK_EQ(cache.size(), 0);
    CHECK_UNARY(cache.empty());
    CHECK_EQ(cache.capacity(), 3);

    /** @arg 测试基本操作序列 */
    cache.insert(1, "one");
    cache.insert(2, "two");
    cache.insert(3, "three");

    CHECK_EQ(cache.size(), 3);
    CHECK_UNARY(!cache.empty());

    /** @arg 测试混合操作 */
    // 先删除一个元素
    CHECK_UNARY(cache.remove(2));
    CHECK_EQ(cache.size(), 2);

    // 再添加一个元素，此时未达到容量上限
    cache.insert(4, "four");
    CHECK_EQ(cache.size(), 3);

    // 验证元素的存在性
    CHECK_UNARY(cache.contains(1));
    CHECK_UNARY(!cache.contains(2));  // 已删除
    CHECK_UNARY(cache.contains(3));
    CHECK_UNARY(cache.contains(4));

    /** @arg 测试clear操作 */
    cache.clear();
    CHECK_EQ(cache.size(), 0);
    CHECK_UNARY(cache.empty());
    CHECK_EQ(cache.capacity(), 3);  // 容量不应该改变

    // 验证所有元素都被清除了
    CHECK_UNARY(!cache.contains(1));
    CHECK_UNARY(!cache.contains(3));
    CHECK_UNARY(!cache.contains(4));

    /** @arg 测试重新填充 */
    cache.insert(5, "five");
    CHECK_EQ(cache.get(5), "five");
    CHECK_EQ(cache.size(), 1);

    /** @arg 测试tryGet与其它操作的交互 */
    std::string value;
    bool found = cache.tryGet(5, value);
    CHECK_UNARY(found);
    CHECK_EQ(value, "five");

    // 尝试获取不存在的键
    found = cache.tryGet(99, value);
    CHECK_UNARY(!found);

    /** @arg 测试resize操作 */
    cache.resize(1);
    CHECK_EQ(cache.capacity(), 1);

    cache.insert(6, "six");
    cache.insert(7, "seven");  // 这个应该导致LRU淘汰

    // 验证只有最新的元素存在
    CHECK_UNARY(!cache.contains(5));  // 早期的元素应该被移除
    CHECK_UNARY(!cache.contains(6));  // 上一个元素应该被移除
    CHECK_UNARY(cache.contains(7));   // 最新的元素应该存在
}

/** @par 检测点: 冗余容量功能测试 */
TEST_CASE("test_LruCache_overflow") {
    LruCache<int, std::string> cache(3, 2);  // 容量为3，溢出容量为2

    /** @arg 测试在容量+溢出容量范围内不发生淘汰 */
    cache.insert(1, "one");
    cache.insert(2, "two");
    cache.insert(3, "three");
    cache.insert(4, "four");  // 超过容量但未超过容量+溢出容量
    cache.insert(5, "five");  // 达到容量+溢出容量

    CHECK_EQ(cache.size(), 5);
    CHECK_EQ(cache.capacity(), 3);
    CHECK_EQ(cache.overflow(), 2);

    // 验证所有元素都存在
    CHECK_UNARY(cache.contains(1));
    CHECK_UNARY(cache.contains(2));
    CHECK_UNARY(cache.contains(3));
    CHECK_UNARY(cache.contains(4));
    CHECK_UNARY(cache.contains(5));

    /** @arg 测试超过容量+溢出容量时发生淘汰 */
    cache.insert(6, "six");  // 超过容量+溢出容量，应该开始淘汰

    CHECK_EQ(cache.size(), 3);  // 仍应等于容量

    // 由于LRU机制，最久未访问的元素(1)应该被移除
    CHECK_UNARY(!cache.contains(1));
    CHECK_UNARY(!cache.contains(2));
    CHECK_UNARY(!cache.contains(3));
    CHECK_UNARY(cache.contains(4));
    CHECK_UNARY(cache.contains(5));
    CHECK_UNARY(cache.contains(6));  // 最新插入的应该存在

    /** @arg 测试溢出容量调整 */
    cache.setOverflow(1);  // 将溢出容量调整为1

    // 此时缓存大小为3，但容量+溢出容量变为4，应该触发清理
    CHECK_EQ(cache.size(), 3);  // 应该减少到容量

    /** @arg 测试resize对溢出容量的影响 */
    LruCache<int, std::string> cache2(2, 1);  // 容量为2，溢出容量为1

    cache2.insert(1, "one");
    cache2.insert(2, "two");
    cache2.insert(3, "three");  // 达到容量+溢出容量

    CHECK_EQ(cache2.size(), 3);

    cache2.resize(1);  // 调整容量为1

    cache2.insert(4, "four");
    cache2.insert(5, "five");  // 超过新容量+溢出容量，应该触发清理

    CHECK_EQ(cache2.size(), 1);  // 应该等于新容量
}

/** @par 检测点: overflow和setOverflow功能测试 */
TEST_CASE("test_LruCache_overflow_functions") {
    LruCache<int, std::string> cache(2, 1);  // 容量为2，溢出容量为1

    /** @arg 测试构造函数中设置溢出容量 */
    CHECK_EQ(cache.capacity(), 2);
    CHECK_EQ(cache.overflow(), 1);

    /** @arg 测试setOverflow函数 */
    cache.setOverflow(3);
    CHECK_EQ(cache.overflow(), 3);

    /** @arg 测试在新溢出容量下操作 */
    cache.insert(1, "one");
    cache.insert(2, "two");
    cache.insert(3, "three");
    cache.insert(4, "four");
    cache.insert(5, "five");  // 2+3=5，应该刚好达到容量+溢出容量

    CHECK_EQ(cache.size(), 5);
    CHECK_UNARY(cache.contains(1));
    CHECK_UNARY(cache.contains(2));
    CHECK_UNARY(cache.contains(3));
    CHECK_UNARY(cache.contains(4));
    CHECK_UNARY(cache.contains(5));

    /** @arg 测试超过新容量+溢出容量时的行为 */
    cache.insert(6, "six");  // 超过容量+溢出容量，应该开始淘汰

    CHECK_EQ(cache.size(), 2);  // 仍应等于容量

    /** @arg 测试将溢出容量设置为0（严格容量控制） */
    LruCache<int, std::string> cache2(2, 5);  // 容量2，溢出容量5
    cache2.insert(1, "one");
    cache2.insert(2, "two");
    cache2.insert(3, "three");
    cache2.insert(4, "four");
    cache2.insert(5, "five");
    cache2.insert(6, "six");  // 未超过2+5，不应淘汰

    CHECK_EQ(cache2.size(), 6);
    CHECK_UNARY(cache2.contains(1));

    cache2.setOverflow(0);             // 设置溢出容量为0
    CHECK_EQ(cache2.size(), 2);        // 应该减少到容量，因为超过了容量+溢出容量(2+0)
    CHECK_UNARY(!cache2.contains(1));  // 最早的应该被淘汰
    CHECK_UNARY(cache2.contains(5));   // 5应该还存在（更近访问）
    CHECK_UNARY(cache2.contains(6));   // 6应该还存在
}

/** @} */