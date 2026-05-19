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
 * @tparam MutexType 互斥锁类型，默认为 std::mutex（线程安全）
 */
template <typename ResourceType, typename MutexType = std::mutex>
class ResourceAsioPool {
private:
    struct WaiterNode {
        std::shared_ptr<net::steady_timer> timer;
        ResourceType *reserved_resource = nullptr;  // 预留的资源指针
        WaiterNode() = default;
    };

public:
    ResourceAsioPool() = delete;
    ResourceAsioPool(const ResourceAsioPool &) = delete;
    ResourceAsioPool &operator=(const ResourceAsioPool &) = delete;

    /**
     * 构造函数
     * @param param 连接参数
     * @param max_count 最大资源上限，0 表示无限制
     */
    explicit ResourceAsioPool(const Parameter &param, size_t max_count = 128,
                              size_t max_waiters = 1000)
    : m_maxCount(max_count),
      m_param(param),
      m_resourceList(max_count == 0 ? 128 : max_count),
      m_maxWaiters(max_waiters > 0 ? max_waiters : 1000),
      m_waiterQueue(m_maxWaiters) {
        if (max_waiters == 0) {
            HKU_WARN("ResourceAsioPool: max_waiters is 0, using default 1000");
        } else if (m_maxWaiters > 10000) {
            double estimated_mb = m_maxWaiters * 150.0 / 1024 / 1024;
            HKU_WARN(
              "ResourceAsioPool: large max_waiters={}, "
              "estimated memory usage at full capacity: ~{:.1f}MB",
              m_maxWaiters, estimated_mb);
        }
    }

    /**
     * 析构函数，释放所有缓存的资源
     */
    virtual ~ResourceAsioPool() {
        // 标记正在析构，阻止新的资源获取和归还
        m_is_destroying.store(true, std::memory_order_release);

        // 取消并清理所有等待者
        WaiterNode *waiter = nullptr;
        while (m_waiterQueue.pop(waiter)) {
            if (waiter) {
                // 尝试取消定时器，判断协程是否还在等待
                std::size_t cancelled_ops = 0;
                if (waiter->timer) {
                    cancelled_ops = waiter->timer->cancel();
                }

                // 只有成功取消时，才安全删除预留资源
                // 如果 cancel 返回 0，说明协程可能已经醒来并使用该资源
                if (cancelled_ops > 0 && waiter->reserved_resource) {
                    delete waiter->reserved_resource;
                    m_count.fetch_sub(1);
                }

                delete waiter;
            }
        }

        // 等待所有活跃资源归还
        // 当 m_count == m_idleCount 时，说明所有资源都已归还到空闲队列
        std::unique_lock<MutexType> lock(m_destroy_mutex);
        m_destroy_cv.wait(lock, [this]() {
            return m_count.load(std::memory_order_relaxed) ==
                   m_idleCount.load(std::memory_order_relaxed);
        });

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
        if (m_maxCount == 0 || m_count.load(std::memory_order_relaxed) < m_maxCount) {
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
                                            m_maxCount, m_count.load(std::memory_order_relaxed)));
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
        if (m_maxCount == 0 || m_count.load(std::memory_order_relaxed) < m_maxCount) {
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
        auto waiter = new WaiterNode();
        waiter->timer = std::make_shared<net::steady_timer>(executor);
        waiter->timer->expires_after(timeout);

        // 无锁加入等待队列
        if (!m_waiterQueue.push(waiter)) {
            delete waiter;
            co_return stdx::unexpected(fmt::format("Waiter queue is full (max={})", m_maxWaiters));
        }

        // 等待被唤醒或超时
        net::error_code ec;
        try {
            co_await waiter->timer->async_wait(net::redirect_error(net::use_awaitable, ec));
        } catch (...) {
            ec = net::error::operation_aborted;
        }

        if (ec == net::error::operation_aborted && waiter->reserved_resource) {
            // 被唤醒且有预留资源，直接使用（保证成功）
            ResourceType *res = waiter->reserved_resource;
            delete waiter;  // 清理节点

            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(res, ResourceCloser(this)));
        } else {
            // 超时, 注意：waiter 仍在队列中，将由归还线程或析构函数清理
            co_return stdx::unexpected(
              fmt::format("ResourceAsioPool get timeout, max_count={}, current_count={}",
                          m_maxCount, m_count.load()));
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

    private:
        ResourceAsioPool *m_pool;
    };

    /** 归还至资源池 */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        if (!p) [[unlikely]] {
            HKU_WARN("ResourceAsioPool::returnResource: nullptr");
            return;
        }

        // 如果正在析构，直接删除资源
        if (m_is_destroying.load(std::memory_order_acquire)) {
            delete p;
            m_count.fetch_sub(1);
            m_destroy_cv.notify_one();
            return;
        }

        // 尝试唤醒一个未超时的等待者（资源预留机制）
        WaiterNode *waiter = nullptr;

        // 循环 pop 直到找到未超时的等待者或队列为空
        while (m_waiterQueue.pop(waiter)) {
            // 先预留资源，再取消定时器（关键：确保唤醒前资源已就绪）
            waiter->reserved_resource = p;

            // 尝试取消定时器，通过返回值判断是否成功
            std::size_t cancelled_ops = waiter->timer->cancel();

            if (cancelled_ops > 0) {
                // 成功取消，协程会被唤醒并直接使用预留的资源
                m_destroy_cv.notify_one();
                return;
            } else {
                // cancel 返回 0，说明定时器已自然到期（超时）
                // 撤销预留，清理该节点，继续找下一个等待者
                waiter->reserved_resource = nullptr;
                delete waiter;
            }
        }

        // 没有有效等待者，放回空闲队列
        if (!m_resourceList.push(p)) {
            // 队列已满，直接删除
            delete p;
            m_count.fetch_sub(1);
            m_destroy_cv.notify_one();
            return;
        }

        m_idleCount.fetch_add(1);
        m_destroy_cv.notify_one();
    }

    std::atomic<size_t> m_count{0};      // 当前活动的资源数
    std::atomic<size_t> m_idleCount{0};  // 当前空闲的资源数
    size_t m_maxCount;                   // 最大资源上限
    Parameter m_param;
    boost::lockfree::queue<ResourceType *> m_resourceList;

    size_t m_maxWaiters;                                 // 运行时逻辑上限：最大等待者数量
    boost::lockfree::queue<WaiterNode *> m_waiterQueue;  // 无锁等待队列
    MutexType m_destroy_mutex;                           // 保护析构等待的条件变量
    std::condition_variable_any m_destroy_cv;            // 用于通知析构函数资源已归还
    std::atomic<bool> m_is_destroying{false};            // 标记是否正在析构
};

/**
 * @brief 带版本的资源池（强制要求资源类型支持版本接口）
 * @details 使用 boost::lockfree::queue 实现无锁空闲队列，支持协程异步获取资源。
 *          当参数发生变化时，自动递增版本号并释放所有空闲的旧版本资源。
 *
 *          **重要约束**：ResourceType 必须实现 getVersion() 和 setVersion(int) 方法。
 *
 * @tparam ResourceType 资源类型，必须实现 getVersion() 和 setVersion(int) 方法
 * @tparam MutexType 互斥锁类型，默认为 std::mutex（线程安全）
 * @ingroup Utilities
 */
template <typename ResourceType, typename MutexType = std::mutex>
class ResourceAsioVersionPool {
private:
    struct WaiterNode {
        std::shared_ptr<net::steady_timer> timer;
        ResourceType *reserved_resource = nullptr;  // 预留的资源指针
        WaiterNode() = default;
    };

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
     * @param max_waiters 最大等待者数量
     */
    explicit ResourceAsioVersionPool(const Parameter &param, size_t max_count = 0,
                                     size_t max_waiters = 1000)
    : m_maxCount(max_count),
      m_param(param),
      m_resourceList(128),
      m_maxWaiters(max_waiters > 0 ? max_waiters : 1000),
      m_waiterQueue(m_maxWaiters) {
        if (max_waiters == 0) {
            HKU_WARN("ResourceAsioVersionPool: max_waiters is 0, using default 1000");
        } else if (m_maxWaiters > 10000) {
            double estimated_mb = m_maxWaiters * 150.0 / 1024 / 1024;
            HKU_WARN(
              "ResourceAsioVersionPool: large max_waiters={}, "
              "estimated memory usage at full capacity: ~{:.1f}MB",
              m_maxWaiters, estimated_mb);
        }
    }

    /**
     * 析构函数，释放所有缓存的资源
     */
    virtual ~ResourceAsioVersionPool() {
        // 标记正在析构，阻止新的资源获取和归还
        m_is_destroying.store(true, std::memory_order_release);

        // 取消并清理所有等待者
        WaiterNode *waiter = nullptr;
        while (m_waiterQueue.pop(waiter)) {
            if (waiter) {
                // 尝试取消定时器，判断协程是否还在等待
                std::size_t cancelled_ops = 0;
                if (waiter->timer) {
                    cancelled_ops = waiter->timer->cancel();
                }

                // 只有成功取消时，才安全删除预留资源
                // 如果 cancel 返回 0，说明协程可能已经醒来并使用该资源
                if (cancelled_ops > 0 && waiter->reserved_resource) {
                    delete waiter->reserved_resource;
                    m_count.fetch_sub(1);
                }

                delete waiter;
            }
        }

        // 等待所有活跃资源归还
        std::unique_lock<MutexType> lock(m_destroy_mutex);
        m_destroy_cv.wait(
          lock, [this]() { return m_count.load(std::memory_order_relaxed) == m_idleCount.load(); });

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
        if (m_maxCount == 0 || m_count.load(std::memory_order_relaxed) < m_maxCount) {
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
                                            m_maxCount, m_count.load(std::memory_order_relaxed)));
    }

    /**
     * 协程方式获取可用资源（带超时）
     * @param timeout 超时时间，默认 5 秒
     * @return awaitable<std::expected<ResourcePtr, std::string>>
     * 可等待的结果，成功时包含资源指针，失败时包含错误信息
     */
    awaitable<stdx::expected<ResourcePtr, std::string>> asyncGet(
      std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
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
        if (m_maxCount == 0 || m_count.load(std::memory_order_relaxed) < m_maxCount) {
            try {
                Parameter current_param;
                int current_version;
                {
                    std::lock_guard<MutexType> lock(m_mutex);
                    current_param = m_param;
                    current_version = m_version.load();
                }

                p = new ResourceType(current_param);
                p->setVersion(current_version);
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
        auto waiter = new WaiterNode();
        waiter->timer = std::make_shared<net::steady_timer>(executor);
        waiter->timer->expires_after(timeout);

        // 无锁加入等待队列
        if (!m_waiterQueue.push(waiter)) {
            delete waiter;
            co_return stdx::unexpected(fmt::format("Waiter queue is full (max={})", m_maxWaiters));
        }

        // 等待被唤醒或超时
        net::error_code ec;
        try {
            co_await waiter->timer->async_wait(net::redirect_error(net::use_awaitable, ec));
        } catch (...) {
            ec = net::error::operation_aborted;
        }

        if (ec == net::error::operation_aborted && waiter->reserved_resource) {
            // 被唤醒且有预留资源，直接使用（保证成功）
            ResourceType *res = waiter->reserved_resource;
            delete waiter;  // 清理节点

            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(res, ResourceCloser(this)));
        } else {
            // 超时
            // 注意：waiter 仍在队列中，将由归还线程或析构函数清理
            co_return stdx::unexpected(
              fmt::format("ResourceAsioVersionPool get timeout, max_count={}, current_count={}",
                          m_maxCount, m_count.load(std::memory_order_relaxed)));
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

    private:
        ResourceAsioVersionPool *m_pool;
    };

    /** 归还至资源池 */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        if (!p) [[unlikely]] {
            HKU_WARN("ResourceAsioVersionPool::returnResource: nullptr");
            return;
        }

        // 如果正在析构，直接删除资源
        if (m_is_destroying.load(std::memory_order_acquire)) {
            delete p;
            m_count.fetch_sub(1);
            m_destroy_cv.notify_one();
            return;
        }

        // 当前归还资源的版本和资源池版本相等，才接受归还
        if (p->getVersion() == m_version.load()) {
            // 尝试唤醒一个未超时的等待者（资源预留机制）
            WaiterNode *waiter = nullptr;

            // 循环 pop 直到找到未超时的等待者或队列为空
            while (m_waiterQueue.pop(waiter)) {
                // 先预留资源，再取消定时器（关键：确保唤醒前资源已就绪）
                waiter->reserved_resource = p;

                // 尝试取消定时器，通过返回值判断是否成功
                std::size_t cancelled_ops = waiter->timer->cancel();

                if (cancelled_ops > 0) {
                    // 成功取消，协程会被唤醒并直接使用预留的资源

                    // waiter 将由协程在获取资源后删除
                    m_destroy_cv.notify_one();
                    return;
                } else {
                    // cancel 返回 0，说明定时器已自然到期（超时）
                    // 撤销预留，清理该节点，继续找下一个等待者
                    waiter->reserved_resource = nullptr;
                    delete waiter;
                }
            }

            // 没有有效等待者，放回空闲队列
            if (!m_resourceList.push(p)) {
                // 队列已满，直接删除
                delete p;
                m_count.fetch_sub(1);
                m_destroy_cv.notify_one();
                return;
            }

            m_idleCount.fetch_add(1);
            m_destroy_cv.notify_one();
        } else {
            delete p;
            m_count.fetch_sub(1);
            m_destroy_cv.notify_one();
        }
    }

    std::atomic<size_t> m_count{0};      // 当前活动的资源数
    std::atomic<size_t> m_idleCount{0};  // 当前空闲的资源数
    size_t m_maxCount;                   // 最大资源上限
    Parameter m_param;
    boost::lockfree::queue<ResourceType *> m_resourceList;
    std::atomic<int> m_version{0};  // 当前资源池版本

    mutable MutexType m_mutex;                           // 保护参数访问的互斥锁
    size_t m_maxWaiters;                                 // 最大等待者数量
    boost::lockfree::queue<WaiterNode *> m_waiterQueue;  // 无锁等待队列
    MutexType m_destroy_mutex;                           // 保护析构等待的条件变量
    std::condition_variable_any m_destroy_cv;            // 用于通知析构函数资源已归还
    std::atomic<bool> m_is_destroying{false};            // 标记是否正在析构
};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_ASIO_POOL_H */
