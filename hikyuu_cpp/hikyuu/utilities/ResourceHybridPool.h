/*
 * ResourceHybridPool.h
 *
 *  Copyright (c) 2025, hikyuu.org
 *
 *  Created on: 2025-05-12
 *      Author: fasiondog
 */
#pragma once
#ifndef HKU_UTILS_RESOURCE_HYBRID_POOL_H
#define HKU_UTILS_RESOURCE_HYBRID_POOL_H

#include <mutex>
#include "ResourceTlsPool.h"
#include "ResourcePool.h"

namespace hku {

/**
 * Hybrid resource pool - it combines the synchronous TLS Pool and the synchronous ResourcePool
 *
 * @details It provides a two-level resource acquisition strategy:
 *          1. The resource is preferentially acquired quickly from the thread local synchronous
 * pool (TLS Pool) (lock free, high performance)
 *          2. If the TLS Pool is unavailable, the resource is acquired from the global shared pool
 *          (ResourcePool) (locked, multi-thread supported)
 *
 *          **Important limitations**:
 *          - It is not suitable for the coroutine environment: the TLS Pool would cause a failed
 *          cross-thread return of the resource when the coroutine thread migrates
 *          - It provides the synchronous interfaces only, the asynchronous methods such as
 *          asyncGet() are not supported
 *          - It is suitable for the traditional multi-thread model only, ensuring that the resource
 *          is acquired and released within the same thread
 *          - For the coroutine environment, it is recommended to use a purely global shared pool
 *          (such as ResourceAsioPool)
 *
 *          This design balances the performance and the flexibility:
 *          - The ordinary synchronous code uses the TLS Pool and gets the best performance (lock
 * free)
 *          - When the TLS Pool resources are exhausted it degrades automatically to the global
 * shared pool (locked but supporting the multi-thread sharing)
 *          - The global pool is protected with std::mutex, ensuring the thread safety
 *
 * @tparam ResourceType the resource type, it must support the constructor
 * ResourceType(const Parameter&)
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
 * // Step 2a: use the default TLS pool size (the template parameter)
 * ResourceHybridPool<MyResource> pool(param, 64);  // The global shared pool has at most 64
 * resources
 *
 * // Step 2b: give the actually used size of the TLS pool at runtime (it cannot exceed the template
 * // parameter)
 * ResourceHybridPool<MyResource, 64> pool(param, 64, 16);  // The TLS pool actually uses 16 and the
 *                                                          // global pool uses 64
 *
 * // Step 3: acquire synchronously (the TLS Pool has the priority)
 * auto resource = pool.get();
 * if (resource) {
 *     resource->doWork();
 * }
 * @endcode
 *
 * @note Two independent resource pool instances are maintained internally
 * @note The TLS Pool is thread local, every thread has an independent instance
 * @note The actually used size of the TLS Pool can be given at construction (through the
 *       max_tls_pool_size parameter), but it cannot exceed the template parameter
 * MAX_TLS_POOL_SIZE_LIMIT
 * @note The size of the Global Pool can also be given at construction, it is not limited by the
 *       template parameter
 * @warning It is not recommended to use it in the coroutine environment; the thread migration of
 * the coroutine would make the TLS Pool resource reuse impossible
 */
template <typename ResourceType, size_t MAX_TLS_POOL_SIZE_LIMIT = 2>
class ResourceHybridPool {
public:
    /** TLS Pool type alias */
    using TlsPoolType = ResourceTlsPool<ResourceType, MAX_TLS_POOL_SIZE_LIMIT>;

    /** Global shared pool type alias (std::mutex is used to support the multi-thread concurrent
     *  access) */
    using GlobalPoolType = ResourcePool<ResourceType>;

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
    explicit ResourceHybridPool(const Parameter &param,
                                size_t max_tls_pool_size = MAX_TLS_POOL_SIZE_LIMIT,
                                size_t max_global_pool_size = 64)
    : m_max_global_pool_size(max_global_pool_size), m_max_tls_pool_size(max_tls_pool_size) {
        // Check and truncate the TLS pool size
        if (m_max_tls_pool_size > MAX_TLS_POOL_SIZE_LIMIT) {
            HKU_WARN("max_tls_pool_size({}) exceeds physical limit ({}), truncated to {}",
                     m_max_tls_pool_size, MAX_TLS_POOL_SIZE_LIMIT, MAX_TLS_POOL_SIZE_LIMIT);
            m_max_tls_pool_size = MAX_TLS_POOL_SIZE_LIMIT;
        }

        // Initialize the default parameters of the TLS Pool
        TlsPoolType::init(param);

        // Set the actually used size of the TLS Pool (maxCount is called after getInstance())
        TlsPoolType::getInstance().maxCount(m_max_tls_pool_size);

        // Create the global shared pool (with the size given at runtime, supporting the
        // multi-thread concurrency)
        m_global_pool = std::make_unique<GlobalPoolType>(param, m_max_global_pool_size);
    }

    /**
     * Destructor
     */
    ~ResourceHybridPool() = default;

    /** The copy is forbidden */
    ResourceHybridPool(const ResourceHybridPool &) = delete;
    ResourceHybridPool &operator=(const ResourceHybridPool &) = delete;

    /** The move is allowed */
    ResourceHybridPool(ResourceHybridPool &&) noexcept = default;
    ResourceHybridPool &operator=(ResourceHybridPool &&) noexcept = default;

    /**
     * Get an available resource; a null pointer is returned when the maximum number of the
     * resources allowed is exceeded
     *
     * @return ResourcePtr the resource pointer, nullptr is returned on failure
     *
     * @note The acquisition strategy:
     *       1. If the TLS Pool is disabled, it is acquired from the global pool directly
     *       2. Otherwise it is preferentially acquired from the TLS Pool (the fast path, lock free)
     *       3. If the TLS Pool fails it degrades automatically to the global shared pool
     *       4. If the global pool also fails, nullptr is returned
     *
     * @exception CreateResourceException the new resource creation may throw an exception
     */
    std::shared_ptr<ResourceType> get() {
        // If the TLS Pool is disabled, acquire it from the global pool directly
        if (m_max_tls_pool_size == 0) {
            return m_global_pool->get();
        }

        // Acquire it from the TLS Pool preferentially (the fast path)
        auto tls_result = TlsPoolType::getInstance().get();
        if (tls_result) {
            return tls_result.value();  // The TLS Pool succeeded, extract the shared_ptr
        }

        // The TLS Pool failed, try to acquire it from the global shared pool
        return m_global_pool->get();  // It may return nullptr or throw CreateResourceException
    }

    /**
     * Get an available resource within the given timeout
     *
     * @param ms_timeout the timeout in milliseconds
     * @return ResourcePtr the resource pointer
     *
     * @note The acquisition strategy:
     *       1. First try to acquire it from the TLS Pool (the fast path, lock free)
     *       2. If the TLS Pool fails it degrades automatically to the global shared pool and waits
     *          for the given time
     *       3. If it times out or the creation fails, GetResourceTimeoutException is thrown
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

        // Acquire it from the TLS Pool preferentially (the fast path)
        auto tls_result = TlsPoolType::getInstance().get();
        if (tls_result) {
            return tls_result.value();  // The TLS Pool succeeded, extract the shared_ptr
        }

        // The TLS Pool failed, try to wait and acquire it from the global shared pool (with a
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
     *       1. First try to acquire it from the TLS Pool (the fast path, lock free)
     *       2. If the TLS Pool fails it degrades automatically to the global shared pool and waits
     *          for an idle resource indefinitely
     *       3. If the resource creation fails, CreateResourceException is thrown
     *
     * @note This method blocks the current thread until a resource is acquired
     * @note Use it with caution: if the resource pool is full and no resource is returned, it would
     *       block forever
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
        auto result = TlsPoolType::getInstance().get();
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
    size_t m_max_global_pool_size{64};  // The maximum number of the resources of the global shared
                                        // pool
    size_t m_max_tls_pool_size{MAX_TLS_POOL_SIZE_LIMIT};  // The maximum number of the resources
                                                          // actually used by the TLS pool (it does
                                                          // not exceed the template parameter)
    std::unique_ptr<GlobalPoolType> m_global_pool;        // The global shared pool instance
};

}  // namespace hku

#endif /* RESOURCE_HYBRID_POOL_H */
