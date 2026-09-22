/*
 * test_LruCache.cpp
 *
 *  Created on: 2026-01-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/utilities/LruCache.h>
#include <memory>
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

/** @par Test point: the basic functionality test */
TEST_CASE("test_LruCache_basic") {
    LruCache<int, std::string> cache(3,
                                     0);  // Capacity 3, overflow 0, keeping the original behavior

    /** @arg Test the insertion and the getting */
    cache.insert(1, "one");
    cache.insert(2, "two");
    cache.insert(3, "three");

    CHECK_EQ(cache.get(1), "one");
    CHECK_EQ(cache.get(2), "two");
    CHECK_EQ(cache.get(3), "three");

    /** @arg Test the LRU elimination */
    cache.insert(4, "four");  // The list is now 4,1,3

    CHECK_EQ(cache.get(1), "one");    // The non-strict LRU mode, it still exists
    CHECK_EQ(cache.get(2), "");       // The non-strict LRU mode, it was eliminated
    CHECK_EQ(cache.get(3), "three");  // It still exists
    CHECK_EQ(cache.get(4), "four");   // The latest inserted

    /** @arg Test that an access updates the LRU order */
    cache.get(4);             // Access 4 and mark it as read
    cache.insert(5, "five");  // The list is now 3,5,4

    CHECK_EQ(cache.get(1), "");       // It was eliminated
    CHECK_EQ(cache.get(3), "three");  // It still exists
    CHECK_EQ(cache.get(4), "four");   // It still exists
    CHECK_EQ(cache.get(5), "five");   // The new insertion

    /** @arg Test the capacity and the size */
    CHECK_EQ(cache.capacity(), 3);
    CHECK_EQ(cache.size(), 3);
    CHECK_UNARY(!cache.empty());

    /** @arg Test updating an existing key */
    cache.insert(1, "two_updated");
    CHECK_EQ(cache.get(1), "two_updated");
}

/** @par Test point: the move semantics test */
TEST_CASE("test_LruCache_move_semantics") {
    LruCache<int, std::string> cache(3, 0);  // Overflow 0, keeping the original behavior

    /** @arg Test insert with the move semantics */
    std::string value = "test_value";
    std::string original_value = value;  // Save the original value

    cache.insert(1, std::move(value));  // Insert with the move semantics

    CHECK_EQ(cache.get(1), original_value);  // Make sure the value is stored correctly
    // The value variable may be empty now, because it was moved

    /** @arg Test updating an existing key with the move semantics */
    std::string new_value = "new_test_value";
    std::string original_new_value = new_value;  // Save the original value

    cache.insert(1, std::move(new_value));       // Update the existing key with the move semantics
    CHECK_EQ(cache.get(1), original_new_value);  // Make sure the value was updated correctly

    /** @arg Test the move semantics with the LRU elimination */
    std::string value2 = "value2";
    std::string original_value2 = value2;
    std::string value3 = "value3";
    std::string original_value3 = value3;
    std::string value4 = "value4";
    std::string original_value4 = value4;

    cache.insert(2, std::move(value2));  // Insert the second value
    cache.insert(3, std::move(value3));  // Insert the third value
    CHECK_EQ(cache.get(2), original_value2);
    CHECK_EQ(cache.get(3), original_value3);

    cache.insert(4, std::move(value4));  // This should trigger the LRU elimination, key 1 removed
    CHECK_EQ(cache.get(4), original_value4);

    // Verify which keys still exist
    // Due to the LRU mechanism the least recently accessed key (probably 1) is removed
    CHECK_EQ(cache.size(), 3);  // Make sure the cache size is correct
}

/** @par Test point: the tryGet functionality test */
TEST_CASE("test_LruCache_tryGet") {
    LruCache<int, std::string> cache(3, 0);  // Overflow 0, keeping the original behavior

    /** @arg Test the behavior of tryGet when the key does not exist */
    std::string value;
    bool found = cache.tryGet(1, value);
    CHECK_UNARY(!found);             // The key does not exist, false should be returned
    CHECK_EQ(value, std::string{});  // The value should be the default constructed one

    /** @arg Test the behavior of tryGet when the key exists */
    cache.insert(1, "one");
    bool found2 = cache.tryGet(1, value);
    CHECK_UNARY(found2);     // The key exists, true should be returned
    CHECK_EQ(value, "one");  // The value should be returned correctly

    /** @arg Test that tryGet updates the LRU order */
    cache.insert(2, "two");
    cache.insert(3, "three");
    CHECK_EQ(cache.size(), 3);  // Make sure the three elements are all there

    std::string value3;
    bool found3 = cache.tryGet(2, value3);  // Access 2 to make it the latest
    CHECK_UNARY(found3);
    CHECK_EQ(value3, "two");

    cache.insert(4, "four");  // The earliest accessed element should be eliminated; 2 was accessed
    std::string value4;
    bool found4 = cache.tryGet(1, value4);
    CHECK_UNARY(!found4);             // 1 should have been eliminated
    CHECK_EQ(cache.get(2), "two");    // 2 should still be there, it was accessed just now
    CHECK_EQ(cache.get(3), "three");  // 3 should still be there
    CHECK_EQ(cache.get(4), "four");   // 4 is the latest inserted
}

/** @par Test point: the boundary case test */
TEST_CASE("test_LruCache_edge_cases") {
    LruCache<int, int> cache(0, 0);  // A cache with the capacity 0 and the overflow capacity 0

    /** @arg Test the case of the capacity 0 (an unlimited capacity) */
    cache.insert(1, 100);
    CHECK_EQ(cache.get(1), 100);  // The stored value is returned, the capacity 0 means unlimited
    CHECK_UNARY(!cache.empty());  // The cache should not be empty, because it has elements

    /** @arg Test the capacity adjustment */
    cache.resize(2);
    cache.insert(1, 100);
    cache.insert(2, 200);
    CHECK_EQ(cache.size(), 2);
    CHECK_EQ(cache.get(1), 100);
    CHECK_EQ(cache.get(2), 200);

    /** @arg Test clear */
    cache.clear();
    CHECK_UNARY(cache.empty());
    CHECK_EQ(cache.size(), 0);
}

/** @par Test point: the contains and remove functionality */
TEST_CASE("test_LruCache_contains_remove") {
    LruCache<std::string, int> cache(3, 0);  // Overflow 0, keeping the original behavior

    /** @arg Test the contains functionality */
    cache.insert("key1", 100);
    cache.insert("key2", 200);
    CHECK_UNARY(cache.contains("key1"));
    CHECK_UNARY(cache.contains("key2"));
    CHECK_UNARY(!cache.contains("key3"));

    /** @arg Test the remove functionality */
    bool removed = cache.remove("key1");
    CHECK_UNARY(removed);
    CHECK_UNARY(!cache.contains("key1"));
    CHECK_EQ(cache.get("key1"), 0);

    /** @arg Test removing a key that does not exist */
    removed = cache.remove("nonexistent");
    CHECK_UNARY(!removed);

    /** @arg Test a new insertion after remove */
    cache.insert("key3", 300);
    cache.insert("key4", 400);
    CHECK_UNARY(cache.contains("key3"));
    CHECK_UNARY(cache.contains("key4"));
    CHECK_UNARY(cache.contains("key2"));  // key2 should still be there, key1 was removed
}

/** @par Test point: the thread safety test */
TEST_CASE("test_LruCache_thread_safety") {
    const int num_threads = 10;
    const int ops_per_thread = 100;
    std::vector<std::thread> threads;

    // The cache is managed with a smart pointer
    auto cache = std::make_shared<LruCache<int, int, std::shared_mutex>>(
      1000,
      0);  // A larger capacity to avoid the LRU elimination in the concurrent test, overflow 0

    // The concurrent write test
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back(
          [=, cache_ptr = cache]() {  // Every thread holds a shared_ptr of the cache
              for (int i = 0; i < ops_per_thread; ++i) {
                  int key = t * ops_per_thread + i;
                  cache_ptr->insert(key, key * 2);
              }
          });
    }

    for (auto& th : threads) {
        th.join();
    }

    // Verify the results of the writes
    for (int t = 0; t < num_threads; ++t) {
        for (int i = 0; i < ops_per_thread; ++i) {
            int key = t * ops_per_thread + i;
            int expected_value = key * 2;
            CHECK_EQ(cache->get(key), expected_value);
        }
    }

    // The concurrent read / write test - a separate scope makes sure these threads finish
    {
        std::atomic<bool> stop_flag(false);
        std::vector<std::thread> rw_threads;

        // For the concurrent read / write test a larger capacity avoids the LRU influence
        cache->setOverflow(0);  // Set the overflow capacity to 0
        cache->resize(2000);    // Adjust the capacity to avoid the LRU influence

        // The write thread
        for (int w = 0; w < 3; ++w) {
            rw_threads.emplace_back(
              [w, cache_ptr = cache,
               &stop_flag]() {  // Every thread holds a shared_ptr of the cache
                  int key_offset = w * 1000;
                  int key = key_offset;
                  while (!stop_flag.load()) {
                      cache_ptr->insert(key % 500,
                                        key * 3);  // The keys are reused within a small range
                      key++;
                      std::this_thread::sleep_for(std::chrono::microseconds(100));
                  }
              });
        }

#if defined(_MSC_VER) && defined(__clang__)
#pragma warning(push)
#pragma warning(disable : 4101)
#pragma clang diagnostic ignored "-Wunused-variable"
#endif
        // The read thread
        for (int r = 0; r < 3; ++r) {
            rw_threads.emplace_back(
              [r, cache_ptr = cache,
               &stop_flag]() {  // Every thread holds a shared_ptr of the cache
                  int key_offset = r * 1000;
                  int key = key_offset;
                  while (!stop_flag.load()) {
                      int val = cache_ptr->get(key % 500);
                      // A specific value is not verified any more, because the concurrent writes
                      // may change it
                      key++;
                      std::this_thread::sleep_for(std::chrono::microseconds(100));
                  }
              });
        }

#if defined(_MSC_VER) && defined(__clang__)
#pragma warning(pop)
#endif

        // Stop after running for a while
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        stop_flag.store(true);

        for (auto& th : rw_threads) {
            th.join();
        }

        // rw_threads leaves the scope and is destroyed here
    }

    // The cache leaves the scope only after all the threads have finished, ensuring the safety
}

/** @par Test point: the comprehensive functionality test */
TEST_CASE("test_LruCache_comprehensive") {
    LruCache<int, std::string> cache(3, 0);  // Capacity 3, overflow 0

    /** @arg Test the initial state */
    CHECK_EQ(cache.size(), 0);
    CHECK_UNARY(cache.empty());
    CHECK_EQ(cache.capacity(), 3);

    /** @arg Test the basic operation sequence */
    cache.insert(1, "one");
    cache.insert(2, "two");
    cache.insert(3, "three");

    CHECK_EQ(cache.size(), 3);
    CHECK_UNARY(!cache.empty());

    /** @arg Test the mixed operations */
    // Delete one element first
    CHECK_UNARY(cache.remove(2));
    CHECK_EQ(cache.size(), 2);

    // Add one more element, the capacity limit has not been reached
    cache.insert(4, "four");
    CHECK_EQ(cache.size(), 3);

    // Verify the existence of the elements
    CHECK_UNARY(cache.contains(1));
    CHECK_UNARY(!cache.contains(2));  // It was deleted
    CHECK_UNARY(cache.contains(3));
    CHECK_UNARY(cache.contains(4));

    /** @arg Test the clear operation */
    cache.clear();
    CHECK_EQ(cache.size(), 0);
    CHECK_UNARY(cache.empty());
    CHECK_EQ(cache.capacity(), 3);  // The capacity should not change

    // Verify that all the elements were cleared
    CHECK_UNARY(!cache.contains(1));
    CHECK_UNARY(!cache.contains(3));
    CHECK_UNARY(!cache.contains(4));

    /** @arg Test the refilling */
    cache.insert(5, "five");
    CHECK_EQ(cache.get(5), "five");
    CHECK_EQ(cache.size(), 1);

    /** @arg Test the interaction of tryGet with the other operations */
    std::string value;
    bool found = cache.tryGet(5, value);
    CHECK_UNARY(found);
    CHECK_EQ(value, "five");

    // Try to get a key that does not exist
    found = cache.tryGet(99, value);
    CHECK_UNARY(!found);

    /** @arg Test the resize operation */
    cache.resize(1);
    CHECK_EQ(cache.capacity(), 1);

    cache.insert(6, "six");
    cache.insert(7, "seven");  // This should cause an LRU elimination

    // Verify that only the latest element exists
    CHECK_UNARY(!cache.contains(5));  // The early element should have been removed
    CHECK_UNARY(!cache.contains(6));  // The previous element should have been removed
    CHECK_UNARY(cache.contains(7));   // The latest element should exist
}

/** @par Test point: the overflow capacity test */
TEST_CASE("test_LruCache_overflow") {
    LruCache<int, std::string> cache(3, 2);  // The capacity is 3 and the overflow capacity is 2

    /** @arg Test that no elimination happens within capacity + overflow */
    cache.insert(1, "one");
    cache.insert(2, "two");
    cache.insert(3, "three");
    cache.insert(4, "four");  // Above the capacity but not above capacity + overflow
    cache.insert(5, "five");  // Reaching capacity + overflow

    CHECK_EQ(cache.size(), 5);
    CHECK_EQ(cache.capacity(), 3);
    CHECK_EQ(cache.overflow(), 2);

    // Verify that all the elements exist
    CHECK_UNARY(cache.contains(1));
    CHECK_UNARY(cache.contains(2));
    CHECK_UNARY(cache.contains(3));
    CHECK_UNARY(cache.contains(4));
    CHECK_UNARY(cache.contains(5));

    /** @arg Test that an elimination happens beyond capacity + overflow */
    cache.insert(6, "six");  // Beyond capacity + overflow, the elimination should start

    CHECK_EQ(cache.size(), 3);  // It should still equal the capacity

    // Due to the LRU mechanism the least recently accessed element (1) should be removed
    CHECK_UNARY(!cache.contains(1));
    CHECK_UNARY(!cache.contains(2));
    CHECK_UNARY(!cache.contains(3));
    CHECK_UNARY(cache.contains(4));
    CHECK_UNARY(cache.contains(5));
    CHECK_UNARY(cache.contains(6));  // The latest inserted should exist

    /** @arg Test the overflow capacity adjustment */
    cache.setOverflow(1);  // Adjust the overflow capacity to 1

    // The size is 3 while capacity + overflow becomes 4, which should trigger a cleanup
    CHECK_EQ(cache.size(), 3);  // It should shrink to the capacity

    /** @arg Test the effect of resize on the overflow capacity */
    LruCache<int, std::string> cache2(2, 1);  // The capacity is 2 and the overflow capacity is 1

    cache2.insert(1, "one");
    cache2.insert(2, "two");
    cache2.insert(3, "three");  // Reaching capacity + overflow

    CHECK_EQ(cache2.size(), 3);

    cache2.resize(1);  // Adjust the capacity to 1

    cache2.insert(4, "four");
    cache2.insert(5, "five");  // Beyond the new capacity + overflow, a cleanup should be triggered

    CHECK_EQ(cache2.size(), 1);  // It should equal the new capacity
}

/** @par Test point: the overflow and setOverflow functionality test */
TEST_CASE("test_LruCache_overflow_functions") {
    LruCache<int, std::string> cache(2, 1);  // The capacity is 2 and the overflow capacity is 1

    /** @arg Test setting the overflow capacity in the constructor */
    CHECK_EQ(cache.capacity(), 2);
    CHECK_EQ(cache.overflow(), 1);

    /** @arg Test the setOverflow function */
    cache.setOverflow(3);
    CHECK_EQ(cache.overflow(), 3);

    /** @arg Test the operations under the new overflow capacity */
    cache.insert(1, "one");
    cache.insert(2, "two");
    cache.insert(3, "three");
    cache.insert(4, "four");
    cache.insert(5, "five");  // 2+3=5, exactly reaching capacity + overflow

    CHECK_EQ(cache.size(), 5);
    CHECK_UNARY(cache.contains(1));
    CHECK_UNARY(cache.contains(2));
    CHECK_UNARY(cache.contains(3));
    CHECK_UNARY(cache.contains(4));
    CHECK_UNARY(cache.contains(5));

    /** @arg Test the behavior beyond the new capacity + overflow */
    cache.insert(6, "six");  // Beyond capacity + overflow, the elimination should start

    CHECK_EQ(cache.size(), 2);  // It should still equal the capacity

    /** @arg Test setting the overflow capacity to 0 (a strict capacity control) */
    LruCache<int, std::string> cache2(2, 5);  // Capacity 2, overflow 5
    cache2.insert(1, "one");
    cache2.insert(2, "two");
    cache2.insert(3, "three");
    cache2.insert(4, "four");
    cache2.insert(5, "five");
    cache2.insert(6, "six");  // Not beyond 2+5, no elimination should happen

    CHECK_EQ(cache2.size(), 6);
    CHECK_UNARY(cache2.contains(1));

    cache2.setOverflow(0);       // Set the overflow capacity to 0
    CHECK_EQ(cache2.size(), 2);  // It shrinks to the capacity, capacity + overflow (2+0) exceeded
    CHECK_UNARY(!cache2.contains(1));  // The earliest should have been eliminated
    CHECK_UNARY(cache2.contains(5));   // 5 should still exist (a more recent access)
    CHECK_UNARY(cache2.contains(6));   // 6 should still exist
}

/** @} */