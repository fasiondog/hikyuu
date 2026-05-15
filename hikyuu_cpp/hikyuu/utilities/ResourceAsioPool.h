/*
 * ResourceAsioPool.h
 *
 *  Copyright (c) 2025, hikyuu.org
 *
 *  Created on: 2025-03-17
 *      Author: fasiondog
 */
#pragma once
#ifndef HKU_UTILS_RESOURCE_ASIO_POOL_H
#define HKU_UTILS_RESOURCE_ASIO_POOL_H

#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <boost/lockfree/queue.hpp>

#include "ResourceVersionTraits.h"
#include "Parameter.h"
#include "Log.h"
#include "net.h"
#include "expected.h"

namespace hku {

using net::awaitable;
using net::co_spawn;
using net::detached;
using net::use_awaitable;
namespace this_coro = net::this_coro;

namespace rap {
class NullLock {
public:
    void lock() {}
    void unlock() {}
    bool try_lock() {
        return true;
    }
};

}  // namespace rap

/**
 * 通用共享资源池 - 适用于协程环境
 * 使用 boost 无锁队列，在协程中异步获取资源
 * @ingroup Utilities
 *
 * @tparam ResourceType 资源类型，必须支持构造函数 ResourceType(const Parameter&)
 * @tparam MutexType 互斥锁类型，默认为 NullLock（适用于单线程 io_context）
 */
template <typename ResourceType, typename MutexType = rap::NullLock>
class ResourceAsioPool {
public:
    ResourceAsioPool() = delete;
    ResourceAsioPool(const ResourceAsioPool &) = delete;
    ResourceAsioPool &operator=(const ResourceAsioPool &) = delete;

    /**
     * 构造函数
     * @param param 连接参数
     * @param max_count 最大资源上限，0 表示无限制
     */
    explicit ResourceAsioPool(const Parameter &param, size_t max_count = 128)
    : m_count(0),
      m_idleCount(0),
      m_maxCount(max_count),
      m_param(param),
      m_resourceList(max_count == 0 ? 128 : max_count) {}

    /**
     * 析构函数，释放所有缓存的资源
     */
    virtual ~ResourceAsioPool() {
        // 清空等待队列，取消所有等待的定时器
        {
            std::lock_guard<MutexType> lock(m_waiterMutex);
            for (auto &timer : m_waiters) {
                if (timer) {
                    timer->cancel();
                }
            }
            m_waiters.clear();
        }

        // 标记正在析构，阻止新的资源获取
        m_is_destroying.store(true);

        // 等待所有活跃资源归还
        // 当 m_count == m_idleCount 时，说明所有资源都已归还到空闲队列
        std::unique_lock<MutexType> lock(m_destroy_mutex);
        m_destroy_cv.wait(lock, [this]() { return m_count.load() == m_idleCount.load(); });

        // 此时所有资源都在空闲队列中，释放它们
        ResourceType *p = nullptr;
        while (m_resourceList.pop(p)) {
            if (p) {
                delete p;
            }
        }
    }

    /** 资源实例指针类型 */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * 同步获取可用资源（不等待，无空闲资源直接返回失败）
     *
     * @return std::expected<ResourcePtr, std::string>
     *         成功时包含资源指针，失败时包含错误信息
     *
     * @note 获取策略：
     *       1. 优先从空闲队列获取资源并检查版本
     *       2. 如果资源版本过旧，销毁该资源并尝试创建新资源
     *       3. 如果无空闲但未达上限，创建新版本资源
     *       4. 如果已达上限且无空闲，立即返回失败（不等待）
     *       5. 如需异步等待，请使用 asyncGet()
     *
     * @example
     * @code
     * auto result = pool.get();
     * if (result) {
     *     auto resource = result.value();
     *     // 资源保证是当前最新版本
     *     resource->doWork();
     * } else {
     *     HKU_ERROR("Failed to get resource: {}", result.error());
     * }
     * @endcode
     */
    stdx::expected<ResourcePtr, std::string> get() {
        // 1. 尝试从空闲队列获取资源
        ResourceType *p = nullptr;
        if (m_resourceList.pop(p)) {
            m_idleCount.fetch_sub(1);
            return stdx::expected<ResourcePtr, std::string>(ResourcePtr(p, ResourceCloser(this)));
        }

        // 2. 无空闲但未达上限 → 创建新资源
        if (m_maxCount == 0 || m_count.load() < m_maxCount.load()) {
            try {
                p = new ResourceType(m_param);
            } catch (const std::exception &e) {
                return stdx::unexpected(
                  std::string(fmt::format("Failed create a new Resource! {}", e.what())));
            } catch (...) {
                return stdx::unexpected(
                  std::string("Failed create a new Resource! Unknown error!"));
            }
            m_count.fetch_add(1);
            return stdx::expected<ResourcePtr, std::string>(ResourcePtr(p, ResourceCloser(this)));
        }

        // 3. 已达上限且无空闲资源 → 直接返回失败
        return stdx::unexpected(fmt::format("No available resource, max_count={}, current_count={}",
                                            m_maxCount.load(), m_count.load()));
    }

    /**
     * 协程方式获取可用资源（带超时）
     * @param timeout 超时时间
     * @return awaitable<std::expected<ResourcePtr, std::string>>
     * 可等待的结果，成功时包含资源指针，失败时包含错误信息
     */
    awaitable<stdx::expected<ResourcePtr, std::string>> asyncGet(
      std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
        // 尝试从空闲队列获取资源
        ResourceType *p = nullptr;
        if (m_resourceList.pop(p)) {
            m_idleCount.fetch_sub(1);
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(p, ResourceCloser(this)));
        }

        // 无空闲但未达上限 → 创建新资源
        if (m_maxCount == 0 || m_count.load() < m_maxCount.load()) {
            try {
                p = new ResourceType(m_param);
            } catch (const std::exception &e) {
                co_return stdx::unexpected(
                  std::string(fmt::format("Failed create a new Resource! {}", e.what())));
            } catch (...) {
                co_return stdx::unexpected(
                  std::string("Failed create a new Resource! Unknown error!"));
            }
            m_count.fetch_add(1);
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(p, ResourceCloser(this)));
        }

        // 已达上限 → 进入等待队列
        auto executor = co_await this_coro::executor;
        auto timer = std::make_shared<net::steady_timer>(executor);
        timer->expires_after(timeout);

        // 加入等待队列
        {
            std::lock_guard<MutexType> lock(m_waiterMutex);
            m_waiters.push_back(timer);
        }

        // 等待被唤醒或超时
        net::error_code ec;
        try {
            co_await timer->async_wait(net::redirect_error(net::use_awaitable, ec));
        } catch (...) {
            ec = net::error::operation_aborted;
        }

        // 从等待队列移除
        {
            std::lock_guard<MutexType> lock(m_waiterMutex);
            m_waiters.remove(timer);
        }

        if (ec == net::error::operation_aborted) {
            // 被唤醒，一定能拿到资源
            if (!m_resourceList.pop(p)) {
                co_return stdx::unexpected(
                  std::string("Unexpected error: no available resource after wakeup"));
            }
            m_idleCount.fetch_sub(1);
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(p, ResourceCloser(this)));
        } else {
            // 超时
            co_return stdx::unexpected(
              fmt::format("ResourceAsioPool get timeout, max_count={}, current_count={}",
                          m_maxCount.load(), m_count.load()));
        }
    }

    /** 当前活动的资源数, 即全部资源数（含空闲及被使用的资源） */
    size_t count() const {
        return m_count.load();
    }

    /**
     * 当前空闲的资源数（精确值）
     * 使用原子计数器跟踪，避免操作队列本身
     */
    size_t idleCount() const {
        return m_idleCount.load();
    }

    /** 释放当前所有的空闲资源 */
    void releaseIdleResource() {
        ResourceType *p = nullptr;
        while (m_resourceList.pop(p)) {
            if (p) {
                m_idleCount.fetch_sub(1);  // 减少空闲计数
                delete p;
                m_count.fetch_sub(1);  // 减少计数

                // 通知析构函数：资源计数已变化
                m_destroy_cv.notify_one();
            }
        }
    }

private:
    std::atomic<size_t> m_count;      // 当前活动的资源数
    std::atomic<size_t> m_idleCount;  // 当前空闲的资源数
    std::atomic<size_t> m_maxCount;   // 最大资源上限
    Parameter m_param;
    boost::lockfree::queue<ResourceType *> m_resourceList;

    class ResourceCloser {
    public:
        explicit ResourceCloser(ResourceAsioPool *pool) : m_pool(pool) {}

        void operator()(ResourceType *conn) {
            if (conn) {
                // 如果绑定了 pool，则归还资源；否则删除
                if (m_pool) {
                    m_pool->returnResource(conn, this);
                } else {
                    delete conn;
                }
            }
        }

        // 解绑资源池
        void unbind() {
            m_pool = nullptr;
        }

    private:
        ResourceAsioPool *m_pool;
    };

    /** 归还至资源池 */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        if (!p) [[unlikely]] {
            HKU_WARN("ResourceAsioPool::returnResource: nullptr");
            return;
        }

        if (!m_resourceList.push(p)) {
            // 队列已满（即最大限制），直接删除
            delete p;
            m_count.fetch_sub(1);

            // 通知析构函数：资源计数已变化
            m_destroy_cv.notify_one();
            return;
        }

        m_idleCount.fetch_add(1);

        // 通知析构函数：可能有资源已完全归还
        m_destroy_cv.notify_one();

        // 唤醒一个等待者
        std::shared_ptr<net::steady_timer> timer;
        {
            std::lock_guard<MutexType> lock(m_waiterMutex);
            if (!m_waiters.empty()) {
                timer = m_waiters.front();
                m_waiters.pop_front();
            }
        }
        if (timer) {
            timer->cancel();
        }
    }

    MutexType m_waiterMutex;                                  // 保护等待队列的互斥锁
    std::list<std::shared_ptr<net::steady_timer>> m_waiters;  // 等待队列
    std::atomic<bool> m_is_destroying{false};                 // 标记是否正在析构
    MutexType m_destroy_mutex;                                // 保护析构等待的条件变量
    std::condition_variable_any m_destroy_cv;                 // 用于通知析构函数资源已归还
};

/**
 * @brief 带版本的资源池（强制要求资源类型支持版本接口）
 * @details 使用 boost::lockfree::queue 实现无锁空闲队列，支持协程异步获取资源。
 *          当参数发生变化时，自动递增版本号并释放所有空闲的旧版本资源。
 *
 *          **重要约束**：ResourceType 必须实现 getVersion() 和 setVersion(int) 方法。
 *
 * @tparam ResourceType 资源类型，必须实现 getVersion() 和 setVersion(int) 方法
 * @tparam MutexType 互斥锁类型，默认 std::mutex
 * @ingroup Utilities
 */
template <typename ResourceType, typename MutexType = std::mutex>
class ResourceAsioVersionPool {
public:
    // 编译期检查：ResourceType 必须支持 getVersion 和 setVersion
    static_assert(hku::detail::has_resource_getVersion_v<ResourceType>,
                  "ResourceType must implement getVersion() method.");
    static_assert(hku::detail::has_resource_setVersion_v<ResourceType>,
                  "ResourceType must implement setVersion(int) method.");

    ResourceAsioVersionPool() = delete;
    ResourceAsioVersionPool(const ResourceAsioVersionPool &) = delete;
    ResourceAsioVersionPool &operator=(const ResourceAsioVersionPool &) = delete;

    /**
     * 构造函数
     * @param param 连接参数
     * @param max_count 最大资源上限，0 表示无限制
     */
    explicit ResourceAsioVersionPool(const Parameter &param, size_t max_count = 0)
    : m_count(0),
      m_idleCount(0),
      m_param(param),
      m_resourceList(128),
      m_version(0),
      m_maxCount(max_count) {}

    /**
     * 析构函数，释放所有缓存的资源
     */
    virtual ~ResourceAsioVersionPool() {
        // 清空等待队列，取消所有等待的定时器
        {
            std::lock_guard<MutexType> lock(m_waiterMutex);
            for (auto &timer : m_waiters) {
                if (timer) {
                    timer->cancel();
                }
            }
            m_waiters.clear();
        }

        // 标记正在析构，阻止新的资源获取
        m_is_destroying.store(true);

        // 等待所有活跃资源归还
        std::unique_lock<MutexType> lock(m_destroy_mutex);
        m_destroy_cv.wait(lock, [this]() { return m_count.load() == m_idleCount.load(); });

        // 此时所有资源都在空闲队列中，释放它们
        ResourceType *p = nullptr;
        while (m_resourceList.pop(p)) {
            if (p) {
                delete p;
            }
        }
    }

    /** 指定参数是否存在 */
    bool haveParam(const std::string &name) {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_param.have(name);
    }

    /** 获取指定参数的值，如参数不存在或类型不匹配抛出异常 */
    template <typename ValueType>
    ValueType getParam(const std::string &name) {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_param.get<ValueType>(name);
    }

    /**
     * @brief 设定指定参数的值，参数仅在生成新的资源时生效
     * @details 在原本存在该参数的情况下，新设定的值类型须和原有参数类型相同，否则将抛出异常
     * @param name 参数名
     * @param value 参数值
     * @exception std::logic_error
     */
    template <typename ValueType>
    void setParam(const std::string &name, const ValueType &value) {
        std::lock_guard<MutexType> lock(m_mutex);
        // 如果参数未实际发送变化，则直接返回
        if (m_param.have(name) && value == m_param.get<ValueType>(name)) {
            return;
        }
        m_param.set<ValueType>(name, value);
        m_version.fetch_add(1);
        releaseIdleResource();  // 释放当前空闲资源，以便新参数值生效
    }

    /**
     * @brief 设置资源参数，参数仅在生成新的资源时生效
     * @param param 参数对象
     */
    void setParameter(const Parameter &param) {
        std::lock_guard<MutexType> lock(m_mutex);
        m_param = param;
        m_version.fetch_add(1);
        releaseIdleResource();  // 释放当前空闲资源，以便新参数值生效
    }

    /**
     * @brief 设置资源参数，参数仅在生成新的资源时生效
     * @param param 参数对象
     */
    void setParameter(Parameter &&param) {
        std::lock_guard<MutexType> lock(m_mutex);
        m_param = std::move(param);
        m_version.fetch_add(1);
        releaseIdleResource();  // 释放当前空闲资源，以便新参数值生效
    }

    /** 获取当前资源池版本 */
    int getVersion() {
        return m_version.load();
    }

    /** 递增当前资源池版本，相当于通知资源池资源版本发生变化 */
    void incVersion(int version) {
        m_version.fetch_add(1);
    }

    /** 资源实例指针类型 */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * 同步获取可用资源（不等待，无空闲资源直接返回失败）
     * @return std::expected<ResourcePtr, std::string>
     * 成功时包含资源指针，失败时包含错误信息
     */
    stdx::expected<ResourcePtr, std::string> get() {
        // 1. 尝试从空闲队列获取资源
        ResourceType *p = nullptr;
        if (m_resourceList.pop(p)) {
            m_idleCount.fetch_sub(1);

            // 检查资源版本，如果版本过旧则销毁
            if (p->getVersion() != m_version.load()) {
                delete p;
                m_count.fetch_sub(1);
                p = nullptr;
            } else {
                return stdx::expected<ResourcePtr, std::string>(
                  ResourcePtr(p, ResourceCloser(this)));
            }
        }

        // 2. 未达上限，创建新资源
        if (m_maxCount == 0 || m_count.load() < m_maxCount.load()) {
            try {
                Parameter current_param;
                {
                    std::lock_guard<MutexType> lock(m_mutex);
                    current_param = m_param;
                }

                p = new ResourceType(current_param);
                p->setVersion(m_version.load());
            } catch (const std::exception &e) {
                return stdx::unexpected(
                  std::string(fmt::format("Failed create a new Resource! {}", e.what())));
            } catch (...) {
                return stdx::unexpected(
                  std::string("Failed create a new Resource! Unknown error!"));
            }

            m_count.fetch_add(1);
            return stdx::expected<ResourcePtr, std::string>(ResourcePtr(p, ResourceCloser(this)));
        }

        // 3. 已达上限且无空闲资源 → 直接返回失败
        return stdx::unexpected(fmt::format("No available resource, max_count={}, current_count={}",
                                            m_maxCount.load(), m_count.load()));
    }

    /**
     * 协程方式获取可用资源
     * @return awaitable<std::expected<ResourcePtr, std::string>>
     * 可等待的结果，成功时包含资源指针，失败时包含错误信息
     */
    awaitable<stdx::expected<ResourcePtr, std::string>> asyncGet() {
        return asyncGet(std::chrono::seconds(3));
    }

    /**
     * 协程方式获取可用资源（带超时）
     * @param timeout 超时时间
     * @return awaitable<std::expected<ResourcePtr, std::string>>
     * 可等待的结果，成功时包含资源指针，失败时包含错误信息
     */
    awaitable<stdx::expected<ResourcePtr, std::string>> asyncGet(
      std::chrono::steady_clock::duration timeout) {
        auto executor = co_await this_coro::executor;

        // 尝试从空闲队列获取资源
        ResourceType *p = nullptr;
        if (m_resourceList.pop(p)) {
            m_idleCount.fetch_sub(1);

            // 检查资源版本，如果版本过旧则销毁，下面会创建新资源
            if (p->getVersion() != m_version.load()) {
                delete p;
                m_count.fetch_sub(1);
                p = nullptr;
            } else {
                co_return stdx::expected<ResourcePtr, std::string>(
                  ResourcePtr(p, ResourceCloser(this)));
            }
        }

        // 未达上限，创建新资源
        if (m_maxCount == 0 || m_count.load() < m_maxCount.load()) {
            try {
                Parameter current_param;
                {
                    std::lock_guard<MutexType> lock(m_mutex);
                    current_param = m_param;
                }

                p = new ResourceType(current_param);
                p->setVersion(m_version.load());
            } catch (const std::exception &e) {
                co_return stdx::unexpected(
                  std::string(fmt::format("Failed create a new Resource! {}", e.what())));
            } catch (...) {
                co_return stdx::unexpected(
                  std::string("Failed create a new Resource! Unknown error!"));
            }

            m_count.fetch_add(1);
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(p, ResourceCloser(this)));
        }

        // 已达上限，进入等待队列
        auto timer = std::make_shared<net::steady_timer>(executor);
        timer->expires_after(timeout);

        // 加入等待队列
        {
            std::lock_guard<MutexType> lock(m_waiterMutex);
            m_waiters.push_back(timer);
        }

        // 等待被唤醒或超时
        net::error_code ec;
        try {
            co_await timer->async_wait(net::redirect_error(net::use_awaitable, ec));
        } catch (...) {
            ec = net::error::operation_aborted;
        }

        // 从等待队列移除
        {
            std::lock_guard<MutexType> lock(m_waiterMutex);
            m_waiters.remove(timer);
        }

        if (ec == net::error::operation_aborted) {
            // 被唤醒，一定能拿到资源
            if (!m_resourceList.pop(p)) {
                co_return stdx::unexpected(
                  std::string("Unexpected error: no available resource after wakeup"));
            }
            m_idleCount.fetch_sub(1);

            // 检查资源版本
            if (p->getVersion() != m_version.load()) {
                delete p;
                m_count.fetch_sub(1);
                co_return stdx::unexpected(std::string("Resource version mismatch after wakeup"));
            }

            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(p, ResourceCloser(this)));
        } else {
            co_return stdx::unexpected(
              fmt::format("ResourceAsioVersionPool get timeout, max_count={}, current_count={}",
                          m_maxCount.load(), m_count.load()));
        }
    }

    /** 当前活动的资源数，即全部资源数（含空闲及被使用的资源） */
    size_t count() const {
        return m_count.load();
    }

    /**
     * 当前空闲的资源数（精确值）
     * 使用原子计数器跟踪，避免操作队列本身
     */
    size_t idleCount() const {
        return m_idleCount.load();
    }

    /** 释放当前所有的空闲资源 */
    void releaseIdleResource() {
        ResourceType *p = nullptr;
        while (m_resourceList.pop(p)) {
            if (p) {
                m_idleCount.fetch_sub(1);  // 减少空闲计数
                delete p;
                m_count.fetch_sub(1);  // 减少计数

                // 通知析构函数：资源计数已变化
                m_destroy_cv.notify_one();
            }
        }
    }

private:
    std::atomic<size_t> m_count;      // 当前活动的资源数
    std::atomic<size_t> m_idleCount;  // 当前空闲的资源数
    Parameter m_param;
    boost::lockfree::queue<ResourceType *> m_resourceList;
    std::atomic<int> m_version;  // 当前资源池版本

    class ResourceCloser {
    public:
        explicit ResourceCloser(ResourceAsioVersionPool *pool) : m_pool(pool) {}

        void operator()(ResourceType *conn) {
            if (conn) {
                // 如果绑定了 pool，则归还资源；否则删除
                if (m_pool) {
                    m_pool->returnResource(conn, this);
                } else {
                    delete conn;
                }
            }
        }

        // 解绑资源池
        void unbind() {
            m_pool = nullptr;
        }

    private:
        ResourceAsioVersionPool *m_pool;
    };

    /** 归还至资源池 */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        if (p) {
            // 当前归还资源的版本和资源池版本相等，才接受归还
            if (p->getVersion() == m_version.load()) {
                if (m_resourceList.push(p)) {
                    m_idleCount.fetch_add(1);

                    // 通知析构函数：可能有资源已完全归还
                    m_destroy_cv.notify_one();
                } else {
                    delete p;
                    m_count.fetch_sub(1);

                    // 通知析构函数：资源计数已变化
                    m_destroy_cv.notify_one();
                }

                // 唤醒一个等待者
                std::shared_ptr<net::steady_timer> timer;
                {
                    std::lock_guard<MutexType> lock(m_waiterMutex);
                    if (!m_waiters.empty()) {
                        timer = m_waiters.front();
                        m_waiters.pop_front();
                    }
                }
                if (timer) {
                    timer->cancel();
                }
            } else {
                delete p;
                m_count.fetch_sub(1);

                // 通知析构函数：资源计数已变化
                m_destroy_cv.notify_one();
            }
        } else {
            m_count.fetch_sub(1);

            // 通知析构函数：资源计数已变化
            m_destroy_cv.notify_one();
        }
    }

    mutable MutexType m_mutex;                                // 保护参数访问的互斥锁
    std::atomic<size_t> m_maxCount;                           // 最大资源上限
    MutexType m_waiterMutex;                                  // 保护等待队列的互斥锁
    std::list<std::shared_ptr<net::steady_timer>> m_waiters;  // 等待队列
    std::atomic<bool> m_is_destroying{false};                 // 标记是否正在析构
    MutexType m_destroy_mutex;                                // 保护析构等待的条件变量
    std::condition_variable_any m_destroy_cv;                 // 用于通知析构函数资源已归还
};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_ASIO_POOL_H */
