/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-01-18
 *      Author: fasiondog
 */

#pragma once
#include <unordered_map>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <optional>

namespace hku {

class NullLock {
public:
    void lock() noexcept {}
    void unlock() noexcept {}
    bool try_lock() noexcept {
        return true;
    }
    void lock_shared() noexcept {}
    void unlock_shared() noexcept {}
    bool try_lock_shared() noexcept {
        return true;
    }
};

/**
 * @brief LRU (Least Recently Used)
 * Cache implementation (not a strictly defined LRU, in order to improve the concurrent reading
 * performance)
 * @tparam KeyType the key type, it must support hashing and equality comparison
 * @tparam ValueType the value type, it must support the copy and move operations
 */
template <typename KeyType, typename ValueType, class Lock = NullLock>
class LruCache final {
public:
    using key_type = KeyType;
    using value_type = ValueType;
    using size_type = size_t;
    typedef Lock lock_type;
    using UniqueGuard = std::unique_lock<lock_type>;
    using SharedGuard = std::shared_lock<lock_type>;

    // Storage structure: the value + an atomic dirty flag (marking whether it has been accessed by
    // get and the LRU order needs to be updated)
    using CacheValue = std::pair<value_type, std::atomic<bool>>;
    using LruList = std::list<key_type>;
    using CacheMap =
      std::unordered_map<key_type, std::pair<typename LruList::iterator, CacheValue>>;

    /**
     * @brief Constructor
     * @param capacity cache capacity, 64 by default; 0 means an unlimited capacity
     * @param overflow overflow capacity, 8 by default; it allows the cache to temporarily exceed
     * the given capacity The eviction mechanism is triggered only when the cache size >= the
     * capacity + the overflow capacity
     */
    explicit LruCache(size_type capacity = 64, size_type overflow = 8)
    : m_capacity(capacity), m_overflow(overflow) {}

    ~LruCache() {
        UniqueGuard lock(m_mutex);
        m_cache.clear();
        m_lru_list.clear();
    }

    /**
     * @brief Insert a key-value pair
     * @param key key
     * @param value value
     */
    void insert(const key_type& key, const value_type& value) {
        UniqueGuard lock(m_mutex);
        _batch_update_dirty_nodes();
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            it->second.second.first = value;
            it->second.second.second.store(false, std::memory_order_relaxed);
            m_lru_list.splice(m_lru_list.begin(), m_lru_list, it->second.first);
        } else {
            m_lru_list.emplace_front(key);
            m_cache.emplace(key, std::make_pair(m_lru_list.begin(),
                                                std::make_pair(value, false)  // The initial dirty
                                                                              // flag is false
                                                ));
            _prune_if_needed();
        }
    }

    /**
     * @brief Insert a key-value pair (the move version)
     * @param key key
     * @param value value (an rvalue reference)
     */
    void insert(const key_type& key, value_type&& value) {
        UniqueGuard lock(m_mutex);
        _batch_update_dirty_nodes();
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            it->second.second.first = std::move(value);
            it->second.second.second.store(false, std::memory_order_relaxed);
            m_lru_list.splice(m_lru_list.begin(), m_lru_list, it->second.first);
        } else {
            m_lru_list.emplace_front(key);
            m_cache.emplace(
              key, std::make_pair(m_lru_list.begin(), std::make_pair(std::move(value), false)));
            _prune_if_needed();
        }
    }

    /**
     * @brief Get the value corresponding to the key
     * @param key key
     * @return the value is returned if it exists, otherwise the default constructed value of
     *         ValueType is returned
     */
    value_type get(const key_type& key) {
        SharedGuard lock(m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            it->second.second.second.store(true, std::memory_order_relaxed);
            return it->second.second.first;
        }
        return value_type{};
    }

    /**
     * @brief Try to get the value corresponding to the key
     * @param key key
     * @param value the reference parameter used to receive the value
     * @return true is returned if the key exists, otherwise false
     */
    bool tryGet(const key_type& key, value_type& value) {
        SharedGuard lock(m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            it->second.second.second.store(true, std::memory_order_relaxed);
            value = it->second.second.first;
            return true;
        }
        return false;
    }

    /**
     * @brief Check whether the given key is contained
     * @param key key
     * @return true is returned if it exists, otherwise false
     */
    bool contains(const key_type& key) {
        SharedGuard lock(m_mutex);
        return m_cache.find(key) != m_cache.end();
    }

    /**
     * @brief Delete the given key
     * @param key the key to be deleted
     * @return true is returned on a successful deletion, false when it does not exist
     */
    bool remove(const key_type& key) {
        UniqueGuard lock(m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            m_lru_list.erase(it->second.first);
            m_cache.erase(it);
            return true;
        }
        return false;
    }

    /**
     * @brief Clear the cache
     */
    void clear() {
        UniqueGuard lock(m_mutex);
        m_cache.clear();
        m_lru_list.clear();
    }

    /**
     * @brief Get the current size of the cache
     * @return the current number of the cache elements
     */
    size_type size() const {
        SharedGuard lock(m_mutex);
        return m_cache.size();
    }

    /**
     * @brief Check whether the cache is empty
     * @return true is returned when it is empty, otherwise false
     */
    bool empty() const {
        SharedGuard lock(m_mutex);
        return m_cache.empty();
    }

    /**
     * @brief Get the cache capacity
     * @return cache capacity
     */
    size_type capacity() const {
        SharedGuard lock(m_mutex);
        return m_capacity;
    }

    /**
     * @brief Get the cache overflow capacity
     * @return cache overflow capacity
     */
    size_type overflow() const {
        SharedGuard lock(m_mutex);
        return m_overflow;
    }

    /**
     * @brief Set the cache capacity
     * @param capacity the new capacity; 0 means an unlimited capacity
     */
    void resize(size_type capacity) {
        UniqueGuard lock(m_mutex);
        m_capacity = capacity;
        _prune_if_needed();
    }

    /**
     * @brief Set the cache overflow capacity
     * @param overflow the new overflow capacity
     */
    void setOverflow(size_type overflow) {
        UniqueGuard lock(m_mutex);
        m_overflow = overflow;
        _prune_if_needed();
    }

private:
    // If the cache is full, the least recently used item is removed
    size_t _prune_if_needed() {
        size_t maxAllowed = m_capacity + m_overflow;
        if (m_capacity == 0 || m_cache.size() <= maxAllowed) {
            return 0;
        }
        size_t count = 0;
        while (m_cache.size() > m_capacity) {
            m_cache.erase(m_lru_list.back());
            m_lru_list.pop_back();
            ++count;
        }
        return count;
    }

    // Update all the dirty nodes in a batch: move them to the head of the list and clear the dirty
    // flags
    void _batch_update_dirty_nodes() {
        // Fix 1: avoid the default construction and use a pointer/reference to track the latest
        // dirty node
        typename LruList::reverse_iterator latest_dirty_it;
        bool has_dirty = false;

        // Traverse the list backward: from the tail to the head, find the first dirty node (the
        // most recently accessed node)
        for (auto it = m_lru_list.rbegin(); it != m_lru_list.rend(); ++it) {
            const key_type& key = *it;
            auto cache_it = m_cache.find(key);
            if (cache_it == m_cache.end()) {
                continue;
            }

            auto& dirty_flag = cache_it->second.second.second;
            // Fix 2: the atomic load judgment avoids an uninitialized access
            if (dirty_flag.load(std::memory_order_relaxed)) {
                latest_dirty_it = it;
                has_dirty = true;
                break;  // Only the most recently accessed dirty node is handled, one is kept at the
                        // tail
            }
        }

        // Only the most recently accessed dirty node is moved to the head (fix 3: the reverse
        // iterator is converted to a forward iterator)
        if (has_dirty) {
            // C++17: convert the reverse iterator to a forward iterator (the base() method)
            auto forward_it = latest_dirty_it.base();
            --forward_it;  // base() of the reverse iterator returns the next forward iterator, it
                           // needs to be decreased by 1

            const key_type& key = *forward_it;
            auto cache_it = m_cache.find(key);
            if (cache_it != m_cache.end()) {
                // Clear the dirty flag
                cache_it->second.second.second.store(false, std::memory_order_relaxed);
                // Move the node to the head of the list (splice supports the forward iterators
                // only)
                m_lru_list.splice(m_lru_list.begin(), m_lru_list, forward_it);
            }
        }
    }

private:
    size_type m_capacity;
    size_type m_overflow;
    LruList m_lru_list;
    CacheMap m_cache;
    mutable lock_type m_mutex;
};

}  // namespace hku