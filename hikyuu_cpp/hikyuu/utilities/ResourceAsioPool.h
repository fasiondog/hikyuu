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

#include <boost/lockfree/queue.hpp>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/awaitable.hpp>
#include <chrono>
#include <vector>
#include <atomic>
#include <unordered_set>
#include "Parameter.h"
#include "Log.h"
#include "ResourcePool.h"

namespace hku {

using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

/**
 * 通用共享资源池 - 适用于协程环境
 * 使用 boost 无锁队列，在协程中异步获取资源
 * @ingroup Utilities
 *
 * @tparam ResourceType 资源类型，必须支持构造函数 ResourceType(const Parameter&)
 */
template <typename ResourceType>
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
        // 将所有已分配资源的 closer 和 pool 解绑
        for (auto iter = m_closer_set.begin(); iter != m_closer_set.end(); ++iter) {
            (*iter)->unbind();
        }

        // 释放所有空闲资源
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
     * 协程方式获取可用资源（带超时）
     * @param timeout 超时时间
     * @return awaitable<ResourcePtr> 可等待的资源指针
     * @exception CreateResourceException 新资源创建可能抛出异常
     */
    awaitable<ResourcePtr> get(
      std::chrono::steady_clock::duration timeout = std::chrono::seconds(3)) {
        auto executor = co_await this_coro::executor;

        // 1. 尝试从空闲队列获取资源
        ResourceType *p = nullptr;
        if (m_resourceList.pop(p)) {
            m_idleCount.fetch_sub(1);
            auto result = ResourcePtr(p, ResourceCloser(this));
            co_return result;
        }

        // 2. 无空闲但未达上限 → 创建新资源
        if (m_maxCount == 0 || m_count.load() < m_maxCount.load()) {
            try {
                p = new ResourceType(m_param);
            } catch (const std::exception &e) {
                HKU_THROW_EXCEPTION(CreateResourceException, "Failed create a new Resource! {}",
                                    e.what());
            } catch (...) {
                HKU_THROW_EXCEPTION(CreateResourceException,
                                    "Failed create a new Resource! Unknown error!");
            }
            m_count.fetch_add(1);
            auto result = ResourcePtr(p, ResourceCloser(this));
            {
                std::lock_guard<std::mutex> lock(m_closer_mutex);
                m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
            }
            co_return result;
        }

        // 3. 已达上限 → 进入等待队列
        auto timer = std::make_shared<boost::asio::steady_timer>(executor);
        timer->expires_after(timeout);

        // 加入等待队列
        {
            std::lock_guard<std::mutex> lock(m_waiterMutex);
            m_waiters.push(timer);
        }

        // 等待被唤醒或超时
        boost::system::error_code ec;
        try {
            co_await timer->async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        } catch (...) {
            ec = boost::asio::error::operation_aborted;
        }

        // 从等待队列移除
        {
            std::lock_guard<std::mutex> lock(m_waiterMutex);
            if (!m_waiters.empty() && m_waiters.front() == timer) {
                m_waiters.pop();
            } else {
                // 如果不是队首，需要查找并移除（超时情况）
                std::queue<std::shared_ptr<boost::asio::steady_timer>> temp;
                while (!m_waiters.empty()) {
                    auto t = m_waiters.front();
                    m_waiters.pop();
                    if (t != timer) {
                        temp.push(t);
                    }
                }
                m_waiters = std::move(temp);
            }
        }

        if (ec == boost::asio::error::operation_aborted) {
            // 被唤醒，一定能拿到资源
            if (!m_resourceList.pop(p)) {
                HKU_THROW_EXCEPTION(CreateResourceException,
                                    "Unexpected error: no available resource after wakeup");
            }
            m_idleCount.fetch_sub(1);
            auto result = ResourcePtr(p, ResourceCloser(this));
            co_return result;
        } else {
            // 超时
            HKU_THROW_EXCEPTION(CreateResourceException,
                                "ResourceAsioPool get timeout, max_count={}, current_count={}",
                                m_maxCount.load(), m_count.load());
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
        if (p) {
            if (m_resourceList.push(p)) {
                m_idleCount.fetch_add(1);
            } else {
                delete p;
                m_count.fetch_sub(1);
            }
        } else {
            m_count.fetch_sub(1);
        }

        // 唤醒一个等待者
        std::shared_ptr<boost::asio::steady_timer> timer;
        {
            std::lock_guard<std::mutex> lock(m_waiterMutex);
            if (!m_waiters.empty()) {
                timer = m_waiters.front();
                m_waiters.pop();
            }
        }
        if (timer) {
            timer->cancel();
        }

        if (closer) {
            std::lock_guard<std::mutex> lock(m_closer_mutex);
            m_closer_set.erase(closer);
        }
    }

    std::mutex m_closer_mutex;                                         // 保护 closer_set 的互斥锁
    std::unordered_set<ResourceCloser *> m_closer_set;                 // 占用资源的 closer
    std::mutex m_waiterMutex;                                          // 保护等待队列的互斥锁
    std::queue<std::shared_ptr<boost::asio::steady_timer>> m_waiters;  // 等待队列
};

/**
 * @brief 带版本的资源接口，可由需要版本管理的资源继承
 * @details 自带的 getVersion 和 setVerion 方法由 ResourceAsioVersionPool 调用，不建议带有其他用途
 */
class AsyncResourceWithVersion {
public:
    /** 默认构造函数 */
    AsyncResourceWithVersion() : m_version(0) {}

    /** 析构函数 */
    virtual ~AsyncResourceWithVersion() {}

    /** 获取资源版本 */
    int getVersion() const {
        return m_version;
    }

    /** 设置资源版本 **/
    void setVersion(int version) {
        m_version = version;
    }

protected:
    int m_version;
};

/**
 * 通用版本的共享资源池（协程版本），当资源池参数变更时，保证新资源使用新参数，老版本的资源在使用完毕后被自动回收
 * @details 要求资源类具备 int getVersion() 和 void setVersion(int) 两个接口函数，建议继承
 * AsyncResourceWithVersion
 * @tparam ResourceType 资源类型，必须支持构造函数 ResourceType(const Parameter&) 且继承
 * AsyncResourceWithVersion
 * @ingroup Utilities
 */
template <typename ResourceType>
class ResourceAsioVersionPool {
public:
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
        // 将所有已分配资源的 closer 和 pool 解绑
        for (auto iter = m_closer_set.begin(); iter != m_closer_set.end(); ++iter) {
            (*iter)->unbind();
        }

        // 释放所有空闲资源
        ResourceType *p = nullptr;
        while (m_resourceList.pop(p)) {
            if (p) {
                delete p;
            }
        }
    }

    /** 指定参数是否存在 */
    bool haveParam(const std::string &name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_param.have(name);
    }

    /** 获取指定参数的值，如参数不存在或类型不匹配抛出异常 */
    template <typename ValueType>
    ValueType getParam(const std::string &name) {
        std::lock_guard<std::mutex> lock(m_mutex);
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
        std::lock_guard<std::mutex> lock(m_mutex);
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
        std::lock_guard<std::mutex> lock(m_mutex);
        m_param = param;
        m_version.fetch_add(1);
        releaseIdleResource();  // 释放当前空闲资源，以便新参数值生效
    }

    /**
     * @brief 设置资源参数，参数仅在生成新的资源时生效
     * @param param 参数对象
     */
    void setParameter(Parameter &&param) {
        std::lock_guard<std::mutex> lock(m_mutex);
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
     * 协程方式获取可用资源
     * @return awaitable<ResourcePtr> 可等待的资源指针
     * @exception CreateResourceException 新资源创建可能抛出异常
     */
    awaitable<ResourcePtr> get() {
        return get(std::chrono::seconds(3));
    }

    /**
     * 协程方式获取可用资源（带超时）
     * @param timeout 超时时间
     * @return awaitable<ResourcePtr> 可等待的资源指针
     * @exception CreateResourceException 新资源创建可能抛出异常
     */
    awaitable<ResourcePtr> get(std::chrono::steady_clock::duration timeout) {
        auto executor = co_await this_coro::executor;

        // 1. 尝试从空闲队列获取资源
        ResourceType *p = nullptr;
        if (m_resourceList.pop(p)) {
            m_idleCount.fetch_sub(1);

            // 检查资源版本，如果版本过旧则销毁，下面会创建新资源
            if (p->getVersion() != m_version.load()) {
                delete p;
                m_count.fetch_sub(1);
                p = nullptr;
            } else {
                co_return ResourcePtr(p, ResourceCloser(this));
            }
        }

        // 2. 未达上限，创建新资源
        if (m_maxCount == 0 || m_count.load() < m_maxCount.load()) {
            try {
                Parameter current_param;
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    current_param = m_param;
                }

                p = new ResourceType(current_param);
                p->setVersion(m_version.load());
            } catch (const std::exception &e) {
                HKU_THROW_EXCEPTION(CreateResourceException, "Failed create a new Resource! {}",
                                    e.what());
            } catch (...) {
                HKU_THROW_EXCEPTION(CreateResourceException,
                                    "Failed create a new Resource! Unknown error!");
            }

            m_count.fetch_add(1);
            auto result = ResourcePtr(p, ResourceCloser(this));
            {
                std::lock_guard<std::mutex> lock(m_closer_mutex);
                m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
            }
            co_return result;
        }

        // 3. 已达上限，进入等待队列
        auto timer = std::make_shared<boost::asio::steady_timer>(executor);
        timer->expires_after(timeout);

        // 加入等待队列
        {
            std::lock_guard<std::mutex> lock(m_waiterMutex);
            m_waiters.push(timer);
        }

        // 等待被唤醒或超时
        boost::system::error_code ec;
        try {
            co_await timer->async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));
        } catch (...) {
            ec = boost::asio::error::operation_aborted;
        }

        // 从等待队列移除
        {
            std::lock_guard<std::mutex> lock(m_waiterMutex);
            if (!m_waiters.empty() && m_waiters.front() == timer) {
                m_waiters.pop();
            } else {
                std::queue<std::shared_ptr<boost::asio::steady_timer>> temp;
                while (!m_waiters.empty()) {
                    auto t = m_waiters.front();
                    m_waiters.pop();
                    if (t != timer) {
                        temp.push(t);
                    }
                }
                m_waiters = std::move(temp);
            }
        }

        if (ec == boost::asio::error::operation_aborted) {
            // 被唤醒，一定能拿到资源
            if (!m_resourceList.pop(p)) {
                HKU_THROW_EXCEPTION(CreateResourceException,
                                    "Unexpected error: no available resource after wakeup");
            }
            m_idleCount.fetch_sub(1);

            // 检查资源版本
            if (p->getVersion() != m_version.load()) {
                delete p;
                m_count.fetch_sub(1);
                // 需要创建新资源，但此时已经没有空闲资源，这里可能需要递归或重新获取
                HKU_THROW_EXCEPTION(CreateResourceException,
                                    "Resource version mismatch after wakeup");
            }

            co_return ResourcePtr(p, ResourceCloser(this));
        } else {
            HKU_THROW_EXCEPTION(
              CreateResourceException,
              "ResourceAsioVersionPool get timeout, max_count={}, current_count={}",
              m_maxCount.load(), m_count.load());
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
                } else {
                    delete p;
                    m_count.fetch_sub(1);
                }

                // 唤醒一个等待者
                std::shared_ptr<boost::asio::steady_timer> timer;
                {
                    std::lock_guard<std::mutex> lock(m_waiterMutex);
                    if (!m_waiters.empty()) {
                        timer = m_waiters.front();
                        m_waiters.pop();
                    }
                }
                if (timer) {
                    timer->cancel();
                }
            } else {
                delete p;
                m_count.fetch_sub(1);
            }
        } else {
            m_count.fetch_sub(1);
        }

        if (closer) {
            std::lock_guard<std::mutex> lock(m_closer_mutex);
            m_closer_set.erase(closer);
        }
    }

    std::mutex m_closer_mutex;                                         // 保护 closer_set 的互斥锁
    std::unordered_set<ResourceCloser *> m_closer_set;                 // 占用资源的 closer
    mutable std::mutex m_mutex;                                        // 保护参数访问的互斥锁
    std::atomic<size_t> m_maxCount;                                    // 最大资源上限
    std::mutex m_waiterMutex;                                          // 保护等待队列的互斥锁
    std::queue<std::shared_ptr<boost::asio::steady_timer>> m_waiters;  // 等待队列
};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_ASIO_POOL_H */
