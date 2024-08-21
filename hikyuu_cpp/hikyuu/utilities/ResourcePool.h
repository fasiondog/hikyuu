/*
 * ResourcePool.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-5
 *      Author: fasiondog
 */
#pragma once
#ifndef HKU_UTILS_RESOURCE_POOL_H
#define HKU_UTILS_RESOURCE_POOL_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <unordered_set>
#include "Parameter.h"
#include "Log.h"

namespace hku {

/**
 * 资源获取超时异常
 */
class GetResourceTimeoutException : public hku::exception {
public:
    GetResourceTimeoutException(const char *msg)
    : hku::exception(fmt::format("GetResourceTimeoutException {}", msg)) {}

    GetResourceTimeoutException(const std::string &msg)
    : hku::exception(fmt::format("GetResourceTimeoutException {}", msg)) {}

    virtual ~GetResourceTimeoutException() {}
};

/**
 * 新资源创建失败异常
 */
class CreateResourceException : public hku::exception {
public:
    CreateResourceException(const char *msg)
    : hku::exception(fmt::format("CreateResourceException {}", msg)) {}

    CreateResourceException(const std::string &msg)
    : hku::exception(fmt::format("CreateResourceException {}", msg)) {}

    virtual ~CreateResourceException() {}
};

/**
 * 通用共享资源池
 * @ingroup Utilities
 */
template <typename ResourceType>
class ResourcePool {
public:
    ResourcePool() = delete;
    ResourcePool(const ResourcePool &) = delete;
    ResourcePool &operator=(const ResourcePool &) = delete;

    /**
     * 构造函数
     * @param param 连接参数
     * @param maxPoolSize 允许的最大共享资源数，为 0 表示不限制
     * @param maxIdleNum 运行的最大空闲资源数，为 0 表示用完即刻释放，无缓存
     */
    explicit ResourcePool(const Parameter &param, size_t maxPoolSize = 0, size_t maxIdleNum = 100)
    : m_maxPoolSize(maxPoolSize), m_maxIdelSize(maxIdleNum), m_count(0), m_param(param) {}

    /**
     * 析构函数，释放所有缓存的资源
     */
    virtual ~ResourcePool() {
        std::unique_lock<std::mutex> lock(m_mutex);

        // 将所有已分配资源的 closer 和 pool 解绑
        for (auto iter = m_closer_set.begin(); iter != m_closer_set.end(); ++iter) {
            (*iter)->unbind();
        }

        // 删除所有空闲资源
        while (!m_resourceList.empty()) {
            ResourceType *p = m_resourceList.front();
            m_resourceList.pop();
            if (p) {
                delete p;
            }
        }
    }

    /** 获取当前允许的最大资源数 */
    size_t maxPoolSize() const {
        return m_maxIdelSize;
    }

    /** 获取当前允许的最大空闲资源数 */
    size_t maxIdleSize() const {
        return m_maxIdelSize;
    }

    /** 设置最大资源数 */
    void maxPoolSize(size_t num) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxPoolSize = num;
    }

    /** 设置允许的最大空闲资源数 */
    void maxIdleSize(size_t num) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxIdelSize = num;
    }

    /** 资源实例指针类型 */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * 获取可用资源，如超出允许的最大资源数将返回空指针
     * @exception CreateResourceException 新资源创建可能抛出异常
     */
    ResourcePtr get() {
        std::lock_guard<std::mutex> lock(m_mutex);
        ResourcePtr result;
        ResourceType *p = nullptr;
        if (m_resourceList.empty()) {
            if (m_maxPoolSize > 0 && m_count >= m_maxPoolSize) {
                return result;
            }
            try {
                p = new ResourceType(m_param);
            } catch (const std::exception &e) {
                HKU_THROW_EXCEPTION(CreateResourceException, "Failed create a new Resource! {}",
                                    e.what());
            } catch (...) {
                HKU_THROW_EXCEPTION(CreateResourceException,
                                    "Failed create a new Resource! Unknown error!");
            }
            m_count++;
            result = ResourcePtr(p, ResourceCloser(this));
            m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
            return result;
        }
        p = m_resourceList.front();
        m_resourceList.pop();
        result = ResourcePtr(p, ResourceCloser(this));
        m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
        return result;
    }

    /**
     * 在指定的超时时间内获取可用资源
     * @param ms_timeout 超时时间，单位毫秒
     * @exception GetResourceTimeoutException, CreateResourceException
     */
    ResourcePtr getWaitFor(uint64_t ms_timeout) {  // NOSONAR
        std::unique_lock<std::mutex> lock(m_mutex);
        ResourcePtr result;
        ResourceType *p = nullptr;
        if (m_resourceList.empty()) {
            if (m_maxPoolSize > 0 && m_count >= m_maxPoolSize) {
                // HKU_TRACE("超出最大资源数，等待空闲资源");
                if (ms_timeout > 0) {
                    if (m_cond.wait_for(lock,
                                        std::chrono::duration<uint64_t, std::milli>(ms_timeout),
                                        [&] { return !m_resourceList.empty(); })) {
                        HKU_CHECK_THROW(!m_resourceList.empty(), GetResourceTimeoutException,
                                        "Failed get resource!");
                    } else {
                        HKU_THROW_EXCEPTION(GetResourceTimeoutException, "Failed get resource!");
                    }
                } else {
                    m_cond.wait(lock, [this] { return !m_resourceList.empty(); });
                }
            } else {
                try {
                    p = new ResourceType(m_param);
                } catch (const std::exception &e) {
                    HKU_THROW_EXCEPTION(CreateResourceException, "Failed create a new Resource! {}",
                                        e.what());
                } catch (...) {
                    HKU_THROW_EXCEPTION(CreateResourceException,
                                        "Failed create a new Resource! Unknown error!");
                }
                m_count++;
                result = ResourcePtr(p, ResourceCloser(this));
                m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
                return result;
            }
        }
        p = m_resourceList.front();
        m_resourceList.pop();
        result = ResourcePtr(p, ResourceCloser(this));
        m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
        return result;
    }

    /**
     * 获取可用资源，如超出允许的最大资源数，将阻塞等待直到获得空闲资源
     * @exception CreateResourceException 新资源创建可能抛出异常
     */
    ResourcePtr getAndWait() {
        return getWaitFor(0);
    }

    /** 当前活动的资源数, 即全部资源数（含空闲及被使用的资源） */
    size_t count() const {
        return m_count;
    }

    /** 当前空闲的资源数 */
    size_t idleCount() const {
        return m_resourceList.size();
    }

    /** 释放当前所有的空闲资源 */
    void releaseIdleResource() {
        std::lock_guard<std::mutex> lock(m_mutex);
        _releaseIdleResourceNoLock();
    }

private:
    void _releaseIdleResourceNoLock() {
        while (!m_resourceList.empty()) {
            ResourceType *p = m_resourceList.front();
            m_resourceList.pop();
            m_count--;
            if (p) {
                delete p;
            }
        }
    }

private:
    size_t m_maxPoolSize;  // 允许的最大共享资源数
    size_t m_maxIdelSize;  // 允许的最大空闲资源数
    size_t m_count;        // 当前活动的资源数
    Parameter m_param;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<ResourceType *> m_resourceList;

    class ResourceCloser {
    public:
        explicit ResourceCloser(ResourcePool *pool) : m_pool(pool) {  // NOSONAR
        }

        void operator()(ResourceType *conn) {  // NOSONAR
            if (conn) {
                // 如果绑定了 pool，则归还资源；否则删除
                if (m_pool) {
                    // HKU_DEBUG("retuan to pool");
                    m_pool->returnResource(conn, this);
                } else {
                    // HKU_DEBUG("delete resource not in pool");
                    delete conn;
                }
            }
        }

        // 解绑资源池
        void unbind() {
            m_pool = nullptr;
        }

    private:
        ResourcePool *m_pool;
    };

    /** 归还至资源池 */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (p) {
            if (m_resourceList.size() < m_maxIdelSize) {
                m_resourceList.push(p);
                m_cond.notify_all();
            } else {
                delete p;
                m_count--;
            }
        } else {
            m_count--;
            // HKU_WARN("Trying to return an empty pointer!");
        }
        if (closer) {
            m_closer_set.erase(closer);  // 移除该 closer
        }
    }

    std::unordered_set<ResourceCloser *> m_closer_set;  // 占用资源的 closer
};

/**
 * @brief 带版本的资源接口，可由需要版本管理的资源继承
 * @details 自带的 getVersion 和 setVerion 方法由 ResourceVersionPool 调用，不建议带有其他用途
 */
class ResourceWithVersion {
public:
    /** 默认构造函数 */
    ResourceWithVersion() : m_version(0) {}

    /** 析构函数 */
    virtual ~ResourceWithVersion() {}

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
 * 通用版本的共享资源池，当资源池参数变更时，保证新资源使用新参数，老版本的资源再使用完毕后被自动回收
 * @details 要求资源类具备 int getVersion() 和 void setVersion(int) 另个接口函数
 * @ingroup Utilities
 */
template <typename ResourceType>
class ResourceVersionPool {
public:
    ResourceVersionPool() = delete;
    ResourceVersionPool(const ResourceVersionPool &) = delete;
    ResourceVersionPool &operator=(const ResourceVersionPool &) = delete;

    /**
     * 构造函数
     * @param param 连接参数
     * @param maxPoolSize 允许的最大共享资源数，为 0 表示不限制
     * @param maxIdleNum 运行的最大空闲资源数，为 0 表示用完即刻释放，无缓存
     */
    explicit ResourceVersionPool(const Parameter &param, size_t maxPoolSize = 0,
                                 size_t maxIdleNum = 100)
    : m_maxPoolSize(maxPoolSize),
      m_maxIdelSize(maxIdleNum),
      m_count(0),
      m_param(param),
      m_version(0) {}

    /**
     * 析构函数，释放所有缓存的资源
     */
    virtual ~ResourceVersionPool() {
        std::unique_lock<std::mutex> lock(m_mutex);

        // 将所有已分配资源的 closer 和 pool 解绑
        for (auto iter = m_closer_set.begin(); iter != m_closer_set.end(); ++iter) {
            (*iter)->unbind();
        }

        // 删除所有空闲资源
        while (!m_resourceList.empty()) {
            ResourceType *p = m_resourceList.front();
            m_resourceList.pop();
            if (p) {
                delete p;
            }
        }
    }

    /** 获取当前允许的最大资源数 */
    size_t maxPoolSize() const {
        return m_maxIdelSize;
    }

    /** 获取当前允许的最大空闲资源数 */
    size_t maxIdleSize() const {
        return m_maxIdelSize;
    }

    /** 设置最大资源数 */
    void maxPoolSize(size_t num) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxPoolSize = num;
    }

    /** 设置允许的最大空闲资源数 */
    void maxIdleSize(size_t num) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxIdelSize = num;
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
        HKU_IF_RETURN(m_param.have(name) && value == m_param.get<ValueType>(name), void());
        m_param.set<ValueType>(name, value);
        m_version++;
        _releaseIdleResourceNoLock();  // 释放当前空闲资源，以便新参数值生效
    }

    /**
     * @brief 设置资源参数，参数仅在生成新的资源时生效
     * @param param 参数对象
     */
    void setParameter(const Parameter &param) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_param = param;
        m_version++;
        _releaseIdleResourceNoLock();  // 释放当前空闲资源，以便新参数值生效
    }

    /**
     * @brief 设置资源参数，参数仅在生成新的资源时生效
     * @param param 参数对象
     */
    void setParameter(Parameter &&param) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_param = std::move(param);
        m_version++;
        _releaseIdleResourceNoLock();  // 释放当前空闲资源，以便新参数值生效
    }

    /** 获取当前资源池版本 */
    int getVersion() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_version;
    }

    /** 递增当前资源池版本，相当于通知资源池资源版本发生变化 */
    void incVersion(int version) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_version++;
    }

    /** 资源实例指针类型 */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * 获取可用资源，如超出允许的最大资源数将返回空指针
     * @exception CreateResourceException 新资源创建可能抛出异常
     */
    ResourcePtr get() {
        std::lock_guard<std::mutex> lock(m_mutex);
        ResourcePtr result;
        ResourceType *p = nullptr;
        if (m_resourceList.empty()) {
            if (m_maxPoolSize > 0 && m_count >= m_maxPoolSize) {
                return result;
            }
            try {
                p = new ResourceType(m_param);
                p->setVersion(m_version);
            } catch (const std::exception &e) {
                HKU_THROW_EXCEPTION(CreateResourceException, "Failed create a new Resource! {}",
                                    e.what());
            } catch (...) {
                HKU_THROW_EXCEPTION(CreateResourceException,
                                    "Failed create a new Resource! Unknown error!");
            }
            m_count++;
            result = ResourcePtr(p, ResourceCloser(this));
            m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
            return result;
        }
        p = m_resourceList.front();
        m_resourceList.pop();
        result = ResourcePtr(p, ResourceCloser(this));
        m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
        return result;
    }

    /**
     * 在指定的超时时间内获取可用资源
     * @param ms_timeout 超时时间，单位毫秒
     * @exception GetResourceTimeoutException, CreateResourceException
     */
    ResourcePtr getWaitFor(uint64_t ms_timeout) {  // NOSONAR
        std::unique_lock<std::mutex> lock(m_mutex);
        ResourcePtr result;
        ResourceType *p = nullptr;
        if (m_resourceList.empty()) {
            if (m_maxPoolSize > 0 && m_count >= m_maxPoolSize) {
                // HKU_TRACE("超出最大资源数，等待空闲资源");
                if (ms_timeout > 0) {
                    if (m_cond.wait_for(lock,
                                        std::chrono::duration<uint64_t, std::milli>(ms_timeout),
                                        [&] { return !m_resourceList.empty(); })) {
                        HKU_CHECK_THROW(!m_resourceList.empty(), GetResourceTimeoutException,
                                        "Failed get resource!");
                    } else {
                        HKU_THROW_EXCEPTION(GetResourceTimeoutException, "Failed get resource!");
                    }
                } else {
                    m_cond.wait(lock, [this] { return !m_resourceList.empty(); });
                }
            } else {
                try {
                    p = new ResourceType(m_param);
                    p->setVersion(m_version);
                } catch (const std::exception &e) {
                    HKU_THROW_EXCEPTION(CreateResourceException, "Failed create a new Resource! {}",
                                        e.what());
                } catch (...) {
                    HKU_THROW_EXCEPTION(CreateResourceException,
                                        "Failed create a new Resource! Unknown error!");
                }
                m_count++;
                result = ResourcePtr(p, ResourceCloser(this));
                m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
                return result;
            }
        }
        p = m_resourceList.front();
        m_resourceList.pop();
        result = ResourcePtr(p, ResourceCloser(this));
        m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
        return result;
    }

    /**
     * 获取可用资源，如超出允许的最大资源数，将阻塞等待直到获得空闲资源
     * @exception CreateResourceException 新资源创建可能抛出异常
     */
    ResourcePtr getAndWait() {
        return getWaitFor(0);
    }

    /** 当前活动的资源数, 即全部资源数（含空闲及被使用的资源） */
    size_t count() const {
        return m_count;
    }

    /** 当前空闲的资源数 */
    size_t idleCount() const {
        return m_resourceList.size();
    }

    /** 释放当前所有的空闲资源 */
    void releaseIdleResource() {
        std::lock_guard<std::mutex> lock(m_mutex);
        _releaseIdleResourceNoLock();
    }

private:
    void _releaseIdleResourceNoLock() {
        while (!m_resourceList.empty()) {
            ResourceType *p = m_resourceList.front();
            m_resourceList.pop();
            m_count--;
            if (p) {
                delete p;
            }
        }
    }

private:
    size_t m_maxPoolSize;  // 允许的最大共享资源数
    size_t m_maxIdelSize;  // 允许的最大空闲资源数
    size_t m_count;        // 当前活动的资源数
    Parameter m_param;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<ResourceType *> m_resourceList;
    int m_version;

    class ResourceCloser {
    public:
        explicit ResourceCloser(ResourceVersionPool *pool) : m_pool(pool) {  // NOSONAR
        }

        void operator()(ResourceType *conn) {  // NOSONAR
            if (conn) {
                // 如果绑定了 pool，则归还资源；否则删除
                if (m_pool) {
                    // HKU_DEBUG("retuan to pool");
                    m_pool->returnResource(conn, this);
                } else {
                    // HKU_DEBUG("delete resource not in pool");
                    delete conn;
                }
            }
        }

        // 解绑资源池
        void unbind() {
            m_pool = nullptr;
        }

    private:
        ResourceVersionPool *m_pool;
    };

    /** 归还至资源池 */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (p) {
            // 当前归还资源的版本和资源池版本相等，且空闲资源列表小于最大空闲资源数时，接受归还的资源
            if (p->getVersion() == m_version && m_resourceList.size() < m_maxIdelSize) {
                m_resourceList.push(p);
                m_cond.notify_all();
            } else {
                delete p;
                m_count--;
            }
        } else {
            m_count--;
            // HKU_WARN("Trying to return an empty pointer!");
        }
        if (closer) {
            m_closer_set.erase(closer);  // 移除该 closer
        }
    }

    std::unordered_set<ResourceCloser *> m_closer_set;  // 占用资源的 closer
};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_POOL_H */