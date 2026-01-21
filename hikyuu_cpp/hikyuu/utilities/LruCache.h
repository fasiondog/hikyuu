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
 * 缓存实现(非严格意义LRU以便提升并发读取性能)
 * @tparam KeyType 键的类型，必须支持哈希和相等比较
 * @tparam ValueType 值的类型，必须支持拷贝和移动操作
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

    // 存储结构：值 + 原子脏标记（标记是否被get访问过，需要更新LRU顺序）
    using CacheValue = std::pair<value_type, std::atomic<bool>>;
    using LruList = std::list<key_type>;
    using CacheMap =
      std::unordered_map<key_type, std::pair<typename LruList::iterator, CacheValue>>;

    /**
     * @brief 构造函数
     * @param capacity 缓存容量，默认为64，0表示无限制容量
     * @param overflow 溢出容量，默认为8，允许缓存临时超出设定容量
     *                 仅当缓存大小 >= 容量+溢出容量时才触发淘汰机制
     */
    explicit LruCache(size_type capacity = 64, size_type overflow = 8)
    : m_capacity(capacity), m_overflow(overflow) {}

    ~LruCache() {
        UniqueGuard lock(m_mutex);
        m_cache.clear();
        m_lru_list.clear();
    }

    /**
     * @brief 插入键值对
     * @param key 键
     * @param value 值
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
                                                std::make_pair(value, false)  // 初始脏标记为false
                                                ));
            _prune_if_needed();
        }
    }

    /**
     * @brief 插入键值对（移动版本）
     * @param key 键
     * @param value 值（右值引用）
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
     * @brief 获取键对应的值
     * @param key 键
     * @return 存在则返回值，否则返回ValueType的默认构造值
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
     * @brief 尝试获取键对应的值
     * @param key 键
     * @param value 用于接收值的引用参数
     * @return 如果键存在返回true，否则返回false
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
     * @brief 检查是否包含指定键
     * @param key 键
     * @return 存在返回true，否则返回false
     */
    bool contains(const key_type& key) {
        SharedGuard lock(m_mutex);
        return m_cache.find(key) != m_cache.end();
    }

    /**
     * @brief 删除指定键
     * @param key 要删除的键
     * @return 成功删除返回true，不存在返回false
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
     * @brief 清空缓存
     */
    void clear() {
        UniqueGuard lock(m_mutex);
        m_cache.clear();
        m_lru_list.clear();
    }

    /**
     * @brief 获取缓存当前大小
     * @return 当前缓存元素数量
     */
    size_type size() const {
        SharedGuard lock(m_mutex);
        return m_cache.size();
    }

    /**
     * @brief 检查缓存是否为空
     * @return 空返回true，否则返回false
     */
    bool empty() const {
        SharedGuard lock(m_mutex);
        return m_cache.empty();
    }

    /**
     * @brief 获取缓存容量
     * @return 缓存容量
     */
    size_type capacity() const {
        SharedGuard lock(m_mutex);
        return m_capacity;
    }

    /**
     * @brief 获取缓存溢出容量
     * @return 缓存溢出容量
     */
    size_type overflow() const {
        SharedGuard lock(m_mutex);
        return m_overflow;
    }

    /**
     * @brief 设置缓存容量
     * @param capacity 新的容量，0表示不限制容量
     */
    void resize(size_type capacity) {
        UniqueGuard lock(m_mutex);
        m_capacity = capacity;
        _prune_if_needed();
    }

    /**
     * @brief 设置缓存溢出容量
     * @param overflow 新的溢出容量
     */
    void setOverflow(size_type overflow) {
        UniqueGuard lock(m_mutex);
        m_overflow = overflow;
        _prune_if_needed();
    }

private:
    // 如果缓存已满，移除最久未使用的项
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

    // 批量更新所有脏节点：移到链表头部，清除脏标记
    void _batch_update_dirty_nodes() {
        // 修复点1：避免默认构造，改用指针/引用追踪最新脏节点
        typename LruList::reverse_iterator latest_dirty_it;
        bool has_dirty = false;

        // 反向遍历链表：从尾部→头部，找第一个脏节点（最新访问的节点）
        for (auto it = m_lru_list.rbegin(); it != m_lru_list.rend(); ++it) {
            const key_type& key = *it;
            auto cache_it = m_cache.find(key);
            if (cache_it == m_cache.end()) {
                continue;
            }

            auto& dirty_flag = cache_it->second.second.second;
            // 修复点2：原子加载判断，避免未初始化访问
            if (dirty_flag.load(std::memory_order_relaxed)) {
                latest_dirty_it = it;
                has_dirty = true;
                break;  // 仅处理最新访问的脏节点，保留1在尾部
            }
        }

        // 仅移动最新访问的脏节点到头部（修复点3：反向迭代器转正向迭代器）
        if (has_dirty) {
            // C++17：反向迭代器转正向迭代器（base()方法）
            auto forward_it = latest_dirty_it.base();
            --forward_it;  // 反向迭代器base()返回的是下一个正向迭代器，需减1

            const key_type& key = *forward_it;
            auto cache_it = m_cache.find(key);
            if (cache_it != m_cache.end()) {
                // 清除脏标记
                cache_it->second.second.second.store(false, std::memory_order_relaxed);
                // 移动节点到链表头部（splice仅支持正向迭代器）
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