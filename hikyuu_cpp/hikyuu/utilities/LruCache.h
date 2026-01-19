/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-01-18
 *      Author: fasiondog
 *
 *  注：该代码由 AI 生成
 */

#pragma once
#include <unordered_map>
#include <list>
#include <mutex>
#include <shared_mutex>

namespace hku {

/*
 * a noop lockable concept that can be used in place of std::mutex
 */
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
 * @brief LRU (Least Recently Used) 缓存实现
 *
 * 该缓存实现基于哈希表和双向链表，能够在O(1)时间内完成插入、删除和访问操作。
 * 当缓存达到容量限制时，会自动淘汰最久未使用的元素。
 *
 * 特性：
 * - 线程安全：使用共享互斥锁支持并发读写
 * - 移动语义：支持值的移动插入，避免不必要的拷贝
 * - 溢出容量：允许缓存临时超出设定容量，仅当超过容量+溢出容量时才触发淘汰
 * - 动态调整：支持运行时调整容量和溢出容量
 *
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
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            // 更新已存在的键
            it->second->second = value;
            m_lru_list.splice(m_lru_list.begin(), m_lru_list, it->second);
        } else {
            m_lru_list.emplace_front(key, value);
            m_cache[key] = m_lru_list.begin();
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
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            // 更新已存在的键
            it->second->second = std::move(value);
            m_lru_list.splice(m_lru_list.begin(), m_lru_list, it->second);
        } else {
            m_lru_list.emplace_front(key, std::move(value));
            m_cache[key] = m_lru_list.begin();
            _prune_if_needed();
        }
    }

    /**
     * @brief 获取键对应的值
     * @param key 键
     * @return 存在则返回值，否则返回ValueType的默认构造值
     */
    value_type get(const key_type& key) {
        UniqueGuard lock(m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            // 移动到列表头部（标记为最近使用）
            m_lru_list.splice(m_lru_list.begin(), m_lru_list, it->second);
            return it->second->second;
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
        UniqueGuard lock(m_mutex);
        auto it = m_cache.find(key);
        if (it != m_cache.end()) {
            // 移动到列表头部（标记为最近使用）
            m_lru_list.splice(m_lru_list.begin(), m_lru_list, it->second);
            value = it->second->second;
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
            m_lru_list.erase(it->second);
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
            m_cache.erase(m_lru_list.back().first);
            m_lru_list.pop_back();
            ++count;
        }
        return count;
    }

private:
    // 存储键值对的双向链表，用于维护访问顺序
    std::list<std::pair<key_type, value_type>> m_lru_list;

    // 哈希表，用于快速查找链表节点
    std::unordered_map<key_type, typename std::list<std::pair<key_type, value_type>>::iterator>
      m_cache;

    // 读写锁，保护并发访问
    mutable lock_type m_mutex;

    // 缓存容量，0表示不限制容量
    size_type m_capacity;

    // 缓存溢出容量，在容量为0时不生效
    size_type m_overflow;
};

}  // namespace hku