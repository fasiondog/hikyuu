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
#include "ResourceVersionTraits.h"

namespace hku {

/**
 * Resource acquisition timeout exception
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
 * New resource creation failure exception
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
 * The resource pool was closed (its destructor ran) while a getter was waiting
 */
class ResourcePoolClosedException : public hku::exception {
public:
    ResourcePoolClosedException(const std::string &msg)
    : hku::exception(fmt::format("ResourcePoolClosedException {}", msg)) {}

    virtual ~ResourcePoolClosedException() {}
};

/**
 * General shared resource pool
 * @ingroup Utilities
 */
template <typename ResourceType>
class ResourcePool {
public:
    ResourcePool() = delete;
    ResourcePool(const ResourcePool &) = delete;
    ResourcePool &operator=(const ResourcePool &) = delete;

    /**
     * Constructor
     * @param param connection parameters
     * @param maxPoolSize the maximum number of the shared resources allowed, 0 means unlimited
     * @param maxIdleNum the maximum number of the idle resources allowed; 0 means releasing
     *                   immediately after use without a cache
     */
    explicit ResourcePool(const Parameter &param, size_t maxPoolSize = 0, size_t maxIdleNum = 100)
    : m_maxPoolSize(maxPoolSize), m_maxIdelSize(maxIdleNum), m_count(0), m_param(param) {}

    /**
     * Destructor, it releases all the cached resources
     */
    virtual ~ResourcePool() {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Mark closed and wake all waiters, then wait until every blocked getter has left so it
        // never touches the pool after destruction
        m_closed = true;
        m_cond.notify_all();
        m_exit_cond.wait(lock, [this] { return m_waiting == 0; });

        // Unbind the closer of all the allocated resources from the pool
        for (auto iter = m_closer_set.begin(); iter != m_closer_set.end(); ++iter) {
            (*iter)->unbind();
        }

        // Delete all the idle resources
        while (!m_resourceList.empty()) {
            ResourceType *p = m_resourceList.front();
            m_resourceList.pop();
            if (p) {
                delete p;
            }
        }
    }

    /** Get the current maximum number of the resources allowed */
    size_t maxPoolSize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_maxPoolSize;
    }

    /** Get the current maximum number of the idle resources allowed */
    size_t maxIdleSize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_maxIdelSize;
    }

    /** Set the maximum number of the resources */
    void maxPoolSize(size_t num) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxPoolSize = num;
    }

    /** Set the maximum number of the idle resources allowed */
    void maxIdleSize(size_t num) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxIdelSize = num;
    }

    /** Resource instance pointer type */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * Get an available resource; a null pointer is returned when the maximum number of the
     * resources allowed is exceeded
     * @exception CreateResourceException the new resource creation may throw an exception
     */
    ResourcePtr get() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_closed) {
            HKU_THROW_EXCEPTION(ResourcePoolClosedException, "Failed get resource!");
        }
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
     * Get an available resource within the given timeout
     * @param ms_timeout the timeout in milliseconds
     * @exception GetResourceTimeoutException, CreateResourceException
     */
    ResourcePtr getWaitFor(uint64_t ms_timeout) {  // NOSONAR
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_closed) {
            HKU_THROW_EXCEPTION(ResourcePoolClosedException, "Failed get resource!");
        }
        ResourcePtr result;
        ResourceType *p = nullptr;
        if (m_resourceList.empty()) {
            if (m_maxPoolSize > 0 && m_count >= m_maxPoolSize) {
                m_waiting++;
                auto pred = [this] { return m_closed || !m_resourceList.empty(); };
                if (ms_timeout > 0) {
                    m_cond.wait_for(lock, std::chrono::duration<uint64_t, std::milli>(ms_timeout),
                                    pred);
                } else {
                    m_cond.wait(lock, pred);
                }
                m_waiting--;
                m_exit_cond.notify_all();
                if (m_closed) {
                    HKU_THROW_EXCEPTION(ResourcePoolClosedException, "Failed get resource!");
                }
                if (ms_timeout > 0 && m_resourceList.empty()) {
                    HKU_THROW_EXCEPTION(GetResourceTimeoutException, "Failed get resource!");
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
     * Get an available resource; it blocks and waits until an idle resource is obtained when the
     * maximum number of the resources allowed is exceeded
     * @exception CreateResourceException the new resource creation may throw an exception
     */
    ResourcePtr getAndWait() {
        return getWaitFor(0);
    }

    /** The number of the currently active resources, i.e. all the resources (including the idle and
     *  the used ones) */
    size_t count() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count;
    }

    /** The current number of the idle resources */
    size_t idleCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_resourceList.size();
    }

    /** Release all the currently idle resources */
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
    size_t m_maxPoolSize;  // The maximum number of the shared resources allowed
    size_t m_maxIdelSize;  // The maximum number of the idle resources allowed
    size_t m_count;        // The number of the currently active resources
    Parameter m_param;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
    std::condition_variable m_exit_cond;  // waiters signal here when closing wakes them
    std::queue<ResourceType *> m_resourceList;
    bool m_closed = false;
    size_t m_waiting = 0;  // getters currently blocked on m_cond

    class ResourceCloser {
    public:
        explicit ResourceCloser(ResourcePool *pool) : m_pool(pool) {  // NOSONAR
        }

        void operator()(ResourceType *conn) {  // NOSONAR
            if (conn) {
                // If the pool is bound, the resource is returned; otherwise it is deleted
                if (m_pool) {
                    // HKU_DEBUG("retuan to pool");
                    m_pool->returnResource(conn, this);
                } else {
                    // HKU_DEBUG("delete resource not in pool");
                    delete conn;
                }
            }
        }

        // Unbind the resource pool
        void unbind() {
            m_pool = nullptr;
        }

    private:
        ResourcePool *m_pool;
    };

    /** Return it to the resource pool */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (p) {
            if (!m_closed && m_resourceList.size() < m_maxIdelSize) {
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
            m_closer_set.erase(closer);  // Remove this closer
        }
    }

    std::unordered_set<ResourceCloser *> m_closer_set;  // The closers occupying the resources
};

/**
 * @brief Versioned resource pool (the resource type is required to support the version interfaces)
 * @details The resource class is required to have the two interface functions int getVersion() and
 *          void setVersion(int).
 *          When the parameters change, the version number is increased automatically and all the
 * idle old version resources are released.
 *
 *          **Important constraint**: ResourceType must implement the getVersion() and
 *          setVersion(int) methods.
 *
 * @tparam ResourceType the resource type, it must implement the getVersion() and setVersion(int)
 *                      methods
 * @ingroup Utilities
 */
template <typename ResourceType>
class ResourceVersionPool {
public:
    // Compile-time check: ResourceType must support getVersion and setVersion
    static_assert(hku::detail::has_resource_getVersion_v<ResourceType>,
                  "ResourceType must implement getVersion() method.");
    static_assert(hku::detail::has_resource_setVersion_v<ResourceType>,
                  "ResourceType must implement setVersion(int) method.");

    ResourceVersionPool() = delete;
    ResourceVersionPool(const ResourceVersionPool &) = delete;
    ResourceVersionPool &operator=(const ResourceVersionPool &) = delete;

    /**
     * Constructor
     * @param param connection parameters
     * @param maxPoolSize the maximum number of the shared resources allowed, 0 means unlimited
     * @param maxIdleNum the maximum number of the idle resources allowed; 0 means releasing
     *                   immediately after use without a cache
     */
    explicit ResourceVersionPool(const Parameter &param, size_t maxPoolSize = 0,
                                 size_t maxIdleNum = 100)
    : m_maxPoolSize(maxPoolSize),
      m_maxIdelSize(maxIdleNum),
      m_count(0),
      m_param(param),
      m_version(0) {}

    /**
     * Destructor, it releases all the cached resources
     */
    virtual ~ResourceVersionPool() {
        std::unique_lock<std::mutex> lock(m_mutex);

        // Mark closed and wake all waiters, then wait until every blocked getter has left so it
        // never touches the pool after destruction
        m_closed = true;
        m_cond.notify_all();
        m_exit_cond.wait(lock, [this] { return m_waiting == 0; });

        // Unbind the closer of all the allocated resources from the pool
        for (auto iter = m_closer_set.begin(); iter != m_closer_set.end(); ++iter) {
            (*iter)->unbind();
        }

        // Delete all the idle resources
        while (!m_resourceList.empty()) {
            ResourceType *p = m_resourceList.front();
            m_resourceList.pop();
            if (p) {
                delete p;
            }
        }
    }

    /** Get the current maximum number of the resources allowed */
    size_t maxPoolSize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_maxPoolSize;
    }

    /** Get the current maximum number of the idle resources allowed */
    size_t maxIdleSize() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_maxIdelSize;
    }

    /** Set the maximum number of the resources */
    void maxPoolSize(size_t num) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxPoolSize = num;
    }

    /** Set the maximum number of the idle resources allowed */
    void maxIdleSize(size_t num) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxIdelSize = num;
    }

    /** Whether the given parameter exists */
    bool haveParam(const std::string &name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_param.have(name);
    }

    /** Get the value of the given parameter; an exception is thrown when the parameter does not
     * exist or the type does not match */
    template <typename ValueType>
    ValueType getParam(const std::string &name) {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_param.get<ValueType>(name);
    }

    /**
     * @brief Set the value of the given parameter; the parameter takes effect only when a new
     * resource is created
     * @details When the parameter already exists, the type of the newly set value must be the same
     * as that of the original parameter, otherwise an exception is thrown
     * @param name parameter name
     * @param value parameter value
     * @exception std::logic_error
     */
    template <typename ValueType>
    void setParam(const std::string &name, const ValueType &value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        // If the parameter has not actually changed, return directly
        HKU_IF_RETURN(m_param.have(name) && value == m_param.get<ValueType>(name), void());
        m_param.set<ValueType>(name, value);
        m_version++;
        _releaseIdleResourceNoLock();  // Release the current idle resources so that the new
                                       // parameter values take effect
    }

    /**
     * @brief Set the resource parameters; they take effect only when a new resource is created
     * @param param the parameter object
     */
    void setParameter(const Parameter &param) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_param = param;
        m_version++;
        _releaseIdleResourceNoLock();  // Release the current idle resources so that the new
                                       // parameter values take effect
    }

    /**
     * @brief Set the resource parameters; they take effect only when a new resource is created
     * @param param the parameter object
     */
    void setParameter(Parameter &&param) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_param = std::move(param);
        m_version++;
        _releaseIdleResourceNoLock();  // Release the current idle resources so that the new
                                       // parameter values take effect
    }

    /** Get the current version of the resource pool */
    int getVersion() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_version;
    }

    /** Increase the current version of the resource pool, equivalent to notifying the resource pool
     *  that the resource version has changed */
    void incVersion(int version) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_version += version;
        // Release the idle resources as setParam/setParameter do: otherwise old-version entries
        // would linger in the cache (get validates the version as a second line of defense)
        _releaseIdleResourceNoLock();
    }

    /** Resource instance pointer type */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * Get an available resource; a null pointer is returned when the maximum number of the
     * resources allowed is exceeded
     * @exception CreateResourceException the new resource creation may throw an exception
     */
    ResourcePtr get() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_closed) {
            HKU_THROW_EXCEPTION(ResourcePoolClosedException, "Failed get resource!");
        }
        ResourcePtr result;
        // Take the first idle resource of the current version; stale entries left by an
        // incVersion call are destroyed, freeing capacity for a fresh one
        ResourceType *p = _popCurrentIdle();
        if (!p) {
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
        }
        result = ResourcePtr(p, ResourceCloser(this));
        m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
        return result;
    }

    /**
     * Get an available resource within the given timeout
     * @param ms_timeout the timeout in milliseconds
     * @exception GetResourceTimeoutException, CreateResourceException
     */
    ResourcePtr getWaitFor(uint64_t ms_timeout) {  // NOSONAR
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_closed) {
            HKU_THROW_EXCEPTION(ResourcePoolClosedException, "Failed get resource!");
        }
        ResourcePtr result;
        ResourceType *p = _popCurrentIdle();
        if (!p) {
            if (m_maxPoolSize > 0 && m_count >= m_maxPoolSize) {
                m_waiting++;
                auto pred = [this] { return m_closed || !m_resourceList.empty(); };
                if (ms_timeout > 0) {
                    m_cond.wait_for(lock, std::chrono::duration<uint64_t, std::milli>(ms_timeout),
                                    pred);
                } else {
                    m_cond.wait(lock, pred);
                }
                m_waiting--;
                m_exit_cond.notify_all();
                if (m_closed) {
                    HKU_THROW_EXCEPTION(ResourcePoolClosedException, "Failed get resource!");
                }
                p = _popCurrentIdle();
                if (!p) {
                    // the wait ended with nothing of the current version to take: keep the
                    // original timeout contract for getWaitFor; for the indefinite getAndWait
                    // discarding stale entries always frees capacity, so hitting the limit here
                    // means no resource can be served and reporting is safer than an empty pop
                    if (ms_timeout > 0 || m_count >= m_maxPoolSize) {
                        HKU_THROW_EXCEPTION(GetResourceTimeoutException, "Failed get resource!");
                    }
                }
            }
            if (!p) {
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
            }
        }
        result = ResourcePtr(p, ResourceCloser(this));
        m_closer_set.insert(std::get_deleter<ResourceCloser>(result));
        return result;
    }

    /**
     * Get an available resource; it blocks and waits until an idle resource is obtained when the
     * maximum number of the resources allowed is exceeded
     * @exception CreateResourceException the new resource creation may throw an exception
     */
    ResourcePtr getAndWait() {
        return getWaitFor(0);
    }

    /** The number of the currently active resources, i.e. all the resources (including the idle and
     *  the used ones) */
    size_t count() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_count;
    }

    /** The current number of the idle resources */
    size_t idleCount() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_resourceList.size();
    }

    /** Release all the currently idle resources */
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

    // Pop the first idle resource carrying the current version, destroying older entries on the
    // way: the version can also be raised through incVersion, and an old-version resource must
    // never be handed out (the sibling version pools validate the version at take time the same
    // way). Must be called with m_mutex held
    ResourceType *_popCurrentIdle() {
        while (!m_resourceList.empty()) {
            ResourceType *p = m_resourceList.front();
            m_resourceList.pop();
            if (p && p->getVersion() == m_version) {
                return p;
            }
            delete p;
            m_count--;
        }
        return nullptr;
    }

private:
    size_t m_maxPoolSize;  // The maximum number of the shared resources allowed
    size_t m_maxIdelSize;  // The maximum number of the idle resources allowed
    size_t m_count;        // The number of the currently active resources
    Parameter m_param;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
    std::condition_variable m_exit_cond;  // waiters signal here when closing wakes them
    std::queue<ResourceType *> m_resourceList;
    bool m_closed = false;
    size_t m_waiting = 0;  // getters currently blocked on m_cond
    int m_version;

    class ResourceCloser {
    public:
        explicit ResourceCloser(ResourceVersionPool *pool) : m_pool(pool) {  // NOSONAR
        }

        void operator()(ResourceType *conn) {  // NOSONAR
            if (conn) {
                // If the pool is bound, the resource is returned; otherwise it is deleted
                if (m_pool) {
                    // HKU_DEBUG("retuan to pool");
                    m_pool->returnResource(conn, this);
                } else {
                    // HKU_DEBUG("delete resource not in pool");
                    delete conn;
                }
            }
        }

        // Unbind the resource pool
        void unbind() {
            m_pool = nullptr;
        }

    private:
        ResourceVersionPool *m_pool;
    };

    /** Return it to the resource pool */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (p) {
            // When the version of the currently returned resource equals the resource pool version
            // and the idle resource list is less than the maximum number of the idle resources, the
            // returned resource is accepted
            if (!m_closed && p->getVersion() == m_version &&
                m_resourceList.size() < m_maxIdelSize) {
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
            m_closer_set.erase(closer);  // Remove this closer
        }
    }

    std::unordered_set<ResourceCloser *> m_closer_set;  // The closers occupying the resources
};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_POOL_H */