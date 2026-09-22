/*
 * ResourceHybridVersionPool.h
 *
 *  Copyright (c) 2025, hikyuu.org
 *
 *  Created on: 2025-05-13
 *      Author: fasiondog
 */
#pragma once
#ifndef HKU_UTILS_RESOURCE_HYBRID_VERSION_POOL_H
#define HKU_UTILS_RESOURCE_HYBRID_VERSION_POOL_H

#include <shared_mutex>
#include "ResourceTlsVersionPool.h"
#include "ResourcePool.h"

namespace hku {

/**
 * @brief Hybrid versioned resource pool (TLS + the global shared pool + the version number
 * management)
 * @details It combines the advantages of the thread local storage and the global shared pool, and
 *          supports the parameter version management.
 *          When the parameters change, the version number is increased automatically and all the
 * idle old version resources are released.
 *
 *          **Important limitations**:
 *          - It is not suitable for the coroutine environment: the TLS Pool would cause a failed
 *          cross-thread return of the resource when the coroutine thread migrates
 *          - It provides the synchronous interfaces only, the asynchronous methods such as
 *          asyncGet() are not supported
 *          - It is suitable for the traditional multi-thread model only, ensuring that the resource
 *          is acquired and released within the same thread
 *          - For the coroutine environment, it is recommended to use a purely global shared pool
 *          (such as ResourceAsioVersionPool)
 *
 *          **Important constraint**: ResourceType must implement the getVersion() and
 *          setVersion(int) methods.
 *
 * @tparam ResourceType the resource type, it must implement the getVersion() and setVersion(int)
 *                      methods
 * @tparam MAX_TLS_POOL_SIZE_LIMIT the upper limit of the TLS pool physical capacity, 2 by default
 * (fixed at compile time, used for the std::array allocation)
 * @ingroup Utilities
 *
 * @par Usage example
 * @code
 * // Step 1: initialize the parameters
 * Parameter param;
 * param.set("host", "localhost");
 * param.set("port", 3306);
 *
 * // Step 2: create the hybrid versioned pool (the global shared pool size can be given at runtime)
 * ResourceHybridVersionPool<MyResource> pool(param, 64);  // The global shared pool has at most 64
 *                                                         // resources
 *
 * // Step 3: acquire synchronously (the TLS Pool has the priority)
 * auto resource = pool.get();
 * if (resource) {
 *     resource->doWork();
 * }
 *
 * // Step 4: modify the parameters dynamically (it triggers the version increment)
 * pool.setParam<std::string>("host", "new_host");
 * // The new parameters are used automatically to create the resource at the next acquisition
 * @endcode
 *
 * @note Two independent resource pool instances are maintained internally
 * @note The TLS Pool is thread local, every thread has an independent instance, whose size can be
 *       given at construction (it cannot exceed the template parameter)
 * @note The Global Pool is globally shared and supports the cross-thread access, its size can be
 *       given at construction
 * @note The version number is global, it is shared by all the threads
 * @warning It is not recommended to use it in the coroutine environment; the thread migration of
 * the coroutine would make the TLS Pool resource reuse impossible
 */
template <typename ResourceType, size_t MAX_TLS_POOL_SIZE_LIMIT = 2>
class ResourceHybridVersionPool {
public:
    /** TLS Pool type alias */
    using TlsPoolType = ResourceTlsVersionPool<ResourceType, MAX_TLS_POOL_SIZE_LIMIT>;

    /** Global shared pool type alias (std::mutex is used to support the multi-thread concurrent
     *  access) */
    using GlobalPoolType = ResourceVersionPool<ResourceType>;

    /**
     * Constructor
     *
     * @param param resource creation parameters
     * @param max_tls_pool_size the maximum number of the resources actually used by the TLS pool,
     * by default equal to the template parameter MAX_TLS_POOL_SIZE_LIMIT this value cannot exceed
     * MAX_TLS_POOL_SIZE_LIMIT, otherwise it is truncated if it is set to 0 the TLS Pool is
     * completely disabled and all the requests are acquired from the global shared pool
     * @param max_global_pool_size the maximum number of the resources of the global shared pool
     *                             (Global Pool), 64 by default
     *
     * @note The actual size of the TLS pool is controlled by max_tls_pool_size, but the capacity of
     *       the underlying array is still MAX_TLS_POOL_SIZE_LIMIT
     * @note If max_tls_pool_size > MAX_TLS_POOL_SIZE_LIMIT it is adjusted automatically to
     *       MAX_TLS_POOL_SIZE_LIMIT
     * @note If max_tls_pool_size == 0 the TLS Pool is disabled and it degrades directly to the
     *       global shared pool
     */
    explicit ResourceHybridVersionPool(const Parameter &param,
                                       size_t max_tls_pool_size = MAX_TLS_POOL_SIZE_LIMIT,
                                       size_t max_global_pool_size = 64)
    : m_param(param),
      m_max_global_pool_size(max_global_pool_size),
      m_max_tls_pool_size(max_tls_pool_size),
      m_version(0) {
        // Check and truncate the TLS pool size
        if (m_max_tls_pool_size > MAX_TLS_POOL_SIZE_LIMIT) {
            HKU_WARN("max_tls_pool_size({}) exceeds physical limit ({}), truncated to {}",
                     m_max_tls_pool_size, MAX_TLS_POOL_SIZE_LIMIT, MAX_TLS_POOL_SIZE_LIMIT);
            m_max_tls_pool_size = MAX_TLS_POOL_SIZE_LIMIT;
        }

        // Initialize the default parameters of the TLS Pool
        TlsPoolType::init(param);

        // Set the actually used size of the TLS Pool
        TlsPoolType::getInstance().maxCount(m_max_tls_pool_size);

        // Create the global shared pool (with the size given at runtime, supporting the
        // multi-thread concurrency)
        m_global_pool = std::make_unique<GlobalPoolType>(m_param, m_max_global_pool_size);
    }

    /**
     * Destructor
     */
    ~ResourceHybridVersionPool() = default;

    /** The copy is forbidden */
    ResourceHybridVersionPool(const ResourceHybridVersionPool &) = delete;
    ResourceHybridVersionPool &operator=(const ResourceHybridVersionPool &) = delete;

    /** The move is allowed */
    ResourceHybridVersionPool(ResourceHybridVersionPool &&) noexcept = default;
    ResourceHybridVersionPool &operator=(ResourceHybridVersionPool &&) noexcept = default;

    /**
     * Set a single parameter (it triggers the version increment)
     *
     * @tparam ValueType the parameter value type
     * @param name parameter name
     * @param value parameter value
     *
     * @note This method will:
     *       1. Update the global parameters
     *       2. Increase the global version number
     *       3. Update the parameters of the global shared pool
     *       4. The parameters of the TLS Pool are lazily updated at the next get()
     *
     * @example
     * @code
     * pool.setParam<std::string>("host", "new_host");
     * pool.setParam<int>("port", 3307);
     * @endcode
     */
    template <typename ValueType>
    void setParam(const std::string &name, const ValueType &value) {
        // Update the global parameters and the version number
        {
            std::lock_guard<std::shared_mutex> lock(m_param_mutex);
            m_param.set<ValueType>(name, value);
        }
        m_version.fetch_add(1, std::memory_order_release);

        // Update the parameters of the global shared pool
        m_global_pool->template setParam<ValueType>(name, value);

        // Note: the parameters of the TLS Pool are lazily updated at the next get()
    }

    /**
     * Replace the parameters as a whole (it triggers the version increment)
     *
     * @param param the new parameter object
     *
     * @note This method will:
     *       1. Replace the global parameters
     *       2. Increase the global version number
     *       3. Update the parameters of the global shared pool
     *       4. The parameters of the TLS Pool are lazily updated at the next get()
     */
    void setParameter(const Parameter &param) {
        {
            std::lock_guard<std::shared_mutex> lock(m_param_mutex);
            m_param = param;
        }
        m_version.fetch_add(1, std::memory_order_release);
        m_global_pool->setParameter(param);
    }

    /**
     * Replace the parameters as a whole (the move semantics, it triggers the version increment)
     *
     * @param param the new parameter object (it will be moved)
     */
    void setParameter(Parameter &&param) {
        {
            std::lock_guard<std::shared_mutex> lock(m_param_mutex);
            m_param = std::move(param);
        }
        m_version.fetch_add(1, std::memory_order_release);
        m_global_pool->setParameter(m_param);
    }

    /**
     * Check whether the parameter exists
     *
     * @param name parameter name
     * @return true if the parameter exists
     */
    bool haveParam(const std::string &name) {
        std::shared_lock<std::shared_mutex> lock(m_param_mutex);
        return m_param.have(name);
    }

    /**
     * Get the parameter value
     *
     * @tparam ValueType the parameter value type
     * @param name parameter name
     * @return parameter value
     * @throws std::exception if the parameter does not exist or the type does not match
     */
    template <typename ValueType>
    ValueType getParam(const std::string &name) {
        std::shared_lock<std::shared_mutex> lock(m_param_mutex);
        return m_param.get<ValueType>(name);
    }

    /**
     * Get the current global version number
     *
     * @return the current version number
     */
    int getVersion() const {
        return m_version.load(std::memory_order_acquire);
    }

    /**
     * Manually increase the version number
     *
     * @note It is usually used when the resources need to be forcibly refreshed from outside
     */
    void incVersion() {
        m_version.fetch_add(1, std::memory_order_release);
        m_global_pool->incVersion(1);
    }

    /**
     * Get an available resource; a null pointer is returned when the maximum number of the
     * resources allowed is exceeded
     *
     * @return ResourcePtr the resource pointer, nullptr is returned on failure
     *
     * @note The acquisition strategy:
     *       1. If the TLS Pool is disabled, it is acquired from the global pool directly
     *       2. Otherwise check whether the TLS Pool version needs to be synchronized
     *       3. If the version is not synchronized, update the version and the parameters of the TLS
     *          Pool first
     *       4. Try to acquire it from the TLS Pool (the fast path, lock free)
     *       5. If the TLS Pool fails it degrades automatically to the global shared pool
     *       6. If the global pool also fails, nullptr is returned
     *
     * @exception CreateResourceException the new resource creation may throw an exception
     */
    std::shared_ptr<ResourceType> get() {
        // If the TLS Pool is disabled, acquire it from the global pool directly
        if (m_max_tls_pool_size == 0) {
            return m_global_pool->get();
        }

        // 1. Check whether the TLS Pool version needs to be synchronized
        auto &tls_pool = TlsPoolType::getInstance();
        int current_version = m_version.load(std::memory_order_acquire);

        if (tls_pool.getVersion() != current_version) {
            // The version does not match, synchronize the parameters and the version
            Parameter current_param;
            {
                std::shared_lock<std::shared_mutex> lock(m_param_mutex);
                current_param = m_param;
            }
            tls_pool.syncVersion(current_version, current_param);
        }

        // 2. Acquire it from the TLS Pool preferentially (the fast path)
        auto tls_result = tls_pool.get();
        if (tls_result) {
            return tls_result.value();  // The TLS Pool succeeded, extract the shared_ptr
        }

        // 3. The TLS Pool failed, try to acquire it from the global shared pool
        return m_global_pool->get();  // It may return nullptr or throw CreateResourceException
    }

    /**
     * Get an available resource within the given timeout
     *
     * @param ms_timeout the timeout in milliseconds
     * @return ResourcePtr the resource pointer
     *
     * @note The acquisition strategy:
     *       1. Check whether the TLS Pool version needs to be synchronized
     *       2. If the version is not synchronized, update the version and the parameters of the TLS
     *          Pool first
     *       3. Try to acquire it from the TLS Pool (the fast path, lock free)
     *       4. If the TLS Pool fails it degrades automatically to the global shared pool and waits
     *          for the given time
     *       5. If it times out or the creation fails, GetResourceTimeoutException is thrown
     *
     * @exception GetResourceTimeoutException timeout or the resources are exhausted
     * @exception CreateResourceException the new resource creation failed
     */
    std::shared_ptr<ResourceType> getWaitFor(uint64_t ms_timeout) {
        // If the TLS Pool is disabled (max_tls_pool_size == 0), wait and acquire it from the global
        // pool directly
        if (m_max_tls_pool_size == 0) {
            return m_global_pool->getWaitFor(ms_timeout);  // It may throw an exception
        }

        // 1. Check whether the TLS Pool version needs to be synchronized
        auto &tls_pool = TlsPoolType::getInstance();
        int current_version = m_version.load(std::memory_order_acquire);

        if (tls_pool.getVersion() != current_version) {
            // The version does not match, synchronize the parameters and the version
            Parameter current_param;
            {
                std::shared_lock<std::shared_mutex> lock(m_param_mutex);
                current_param = m_param;
            }
            tls_pool.syncVersion(current_version, current_param);
        }

        // 2. Acquire it from the TLS Pool preferentially (the fast path)
        auto tls_result = tls_pool.get();
        if (tls_result) {
            return tls_result.value();  // The TLS Pool succeeded, extract the shared_ptr
        }

        // 3. The TLS Pool failed, try to wait and acquire it from the global shared pool (with a
        // timeout)
        return m_global_pool->getWaitFor(ms_timeout);  // It may throw GetResourceTimeoutException
    }

    /**
     * Get an available resource; if the maximum number of the resources allowed is exceeded, it
     * blocks and waits until an idle resource is obtained
     *
     * @return ResourcePtr the resource pointer
     *
     * @note The acquisition strategy:
     *       1. Check whether the TLS Pool version needs to be synchronized
     *       2. If the version is not synchronized, update the version and the parameters of the TLS
     *          Pool first
     *       3. Try to acquire it from the TLS Pool (the fast path, lock free)
     *       4. If the TLS Pool fails it degrades automatically to the global shared pool and waits
     *          for an idle resource indefinitely
     *       5. If the resource creation fails, CreateResourceException is thrown
     *
     * @note This method blocks the current thread until a resource is acquired
     * @note Warning: if the resource pool is full and no resource is returned, it would block
     *       forever
     * @note It is suitable for the scenarios where a resource must be acquired, but it must be
     *       guaranteed that the resource will be returned finally
     *
     * @exception GetResourceTimeoutException theoretically it does not time out (ms_timeout=0), but
     * it may be thrown for other reasons
     * @exception CreateResourceException the new resource creation failed
     */
    std::shared_ptr<ResourceType> getAndWait() {
        return getWaitFor(0);  // getWaitFor(0) is called to implement the indefinite waiting
    }

    /**
     * Acquire the resource from the TLS Pool only (synchronously)
     *
     * @return ResourcePtr the resource pointer, nullptr is returned on failure
     */
    std::shared_ptr<ResourceType> getFromTlsPool() {
        // Synchronize the version first
        auto &tls_pool = TlsPoolType::getInstance();
        int current_version = m_version.load(std::memory_order_acquire);

        if (tls_pool.getVersion() != current_version) {
            Parameter current_param;
            {
                std::shared_lock<std::shared_mutex> lock(m_param_mutex);
                current_param = m_param;
            }
            tls_pool.syncVersion(current_version, current_param);
        }

        auto result = tls_pool.get();
        return result ? result.value() : nullptr;
    }

    /**
     * Acquire the resource from the global shared pool only (synchronously)
     *
     * @return ResourcePtr the resource pointer, nullptr is returned on failure
     *
     * @note This method may throw CreateResourceException
     */
    std::shared_ptr<ResourceType> getFromGlobalPool() {
        return m_global_pool->get();  // It may return nullptr or throw CreateResourceException
    }

    /** Get the TLS Pool reference */
    TlsPoolType &tlsPool() {
        return TlsPoolType::getInstance();
    }

    /** Get the global shared pool reference */
    GlobalPoolType &globalPool() {
        return *m_global_pool;
    }

    /** Get the maximum number of the resources actually used by the TLS pool */
    size_t maxTlsPoolSize() const {
        return m_max_tls_pool_size;
    }

    /** Get the maximum number of the resources of the global shared pool */
    size_t maxGlobalPoolSize() const {
        return m_max_global_pool_size;
    }

private:
    Parameter m_param;                  // Global parameters (protected by the lock)
    size_t m_max_global_pool_size{64};  // The maximum number of the resources of the
                                        // global shared pool
    size_t m_max_tls_pool_size{MAX_TLS_POOL_SIZE_LIMIT};  // The maximum number of the resources
                                                          // actually used by the TLS Pool
    std::unique_ptr<GlobalPoolType> m_global_pool;        // The global shared pool instance
    std::atomic<int> m_version{0};                        // Global version number
    mutable std::shared_mutex m_param_mutex;  // The mutex protecting the parameter access
};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_HYBRID_VERSION_POOL_H */