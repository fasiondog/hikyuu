#pragma once
#ifndef HKU_UTILS_RESOURCE_TLS_POOL_H
#define HKU_UTILS_RESOURCE_TLS_POOL_H

#include <array>
#include <memory>
#include <optional>
#include <chrono>
#include <type_traits>
#include "expected.h"
#include "Parameter.h"
#include "Log.h"
#include "exception.h"

/**
 * Thread local resource pool - a completely lock free design (the Ring Buffer implementation)
 *
 * @details It uses the thread_local storage, every thread has an independent resource pool
 * instance. A fixed array + Ring Buffer is used internally to manage the idle resources, avoiding
 * the dynamic memory allocation and the lock contention. Since the resources are completely
 * isolated inside the thread, no lock or atomic operation is needed and the performance is optimal.
 *
 *          **Core features**:
 *          - Completely lock free: based on thread_local, no synchronization mechanism is needed
 *          - Ring Buffer: a fixed array is used to implement the circular queue, the acquisition
 * and the return of the resources have an O(1) time complexity
 *          - Zero dynamic allocation: the capacity is determined at compile time, no malloc/free is
 *          needed at runtime
 *          - Thread isolation: every thread has an independent instance, avoiding the cross-thread
 *          contention
 *
 *          **Important limitations**:
 *          - It is not suitable for the coroutine environment: a coroutine may be migrated between
 *          different threads, causing the resource to be returned across the threads
 *          - On a cross-thread return the resource is deleted directly and a warning log is output,
 *          so the resource reuse cannot be achieved
 *          - If a resource pool is needed in the coroutine environment, it is recommended to use
 * the global shared pool (such as ResourceAsioPool)
 *          - It is suitable for the traditional multi-thread model only, ensuring that the resource
 * is acquired and released within the same thread
 *
 *          **Cross-thread safety check mechanism**:
 *          - The owner thread ID is recorded when the resource is acquired
 *          - Whether the current thread is the same as the owner thread is checked when the
 * resource is returned
 *          - If they are not the same: the resource is deleted directly and a warning log is
 * output, avoiding the undefined behavior caused by accessing the thread_local variables
 *          - If they are the same: it is returned to the pool normally to achieve the reuse
 *
 * @tparam ResourceType the resource type, it must support the constructor
 * ResourceType(const Parameter&)
 * @tparam MAX_POOL_SIZE_LIMIT the upper limit of the physical capacity (fixed at compile time), 2
 * by default
 * @ingroup Utilities
 *
 * @par Usage example
 * @code
 * // Step 1: initialize the default parameters (called once at the program startup)
 * Parameter param;
 * param.set("host", "localhost");
 * param.set("port", 3306);
 * ResourceTlsPool<MyResource>::init(param);
 *
 * // Step 2: get the thread local resource pool instance
 * auto& pool = ResourceTlsPool<MyResource>::getInstance();
 *
 * // Step 3a: acquire a resource synchronously (the recommended usage)
 * auto resource = pool.get();
 * if (resource) {
 *     resource->doWork();
 * }  // It is returned to the pool automatically when leaving the scope
 *
 * // Step 3b: create an independent resource (not managed by the pool)
 * auto standalone = pool.createStandalone();
 * if (standalone) {
 *     standalone.value()->doWork();
 * }  // It is deleted automatically when leaving the scope
 * @endcode
 *
 * @note The constructor is private, it must be used through the init() + getInstance() pattern
 * @note Every thread has an independent resource pool instance, the resources are not shared
 * between the threads
 * @note A Ring Buffer is used internally to manage the idle resource pointers, avoiding the dynamic
 *       expansion of std::vector
 * @note The asynchronous methods such as asyncGet() are not supported, because the TLS resource
 *       acquisition itself is synchronous and fast
 * @warning It is not recommended to use it in the coroutine environment; the thread migration of
 * the coroutine would make the resource reuse impossible
 */
namespace hku {

template <typename ResourceType, size_t MAX_POOL_SIZE_LIMIT = 2>
class ResourceTlsPool {
public:
    /**
     * Initialize the global default parameters (optional)
     *
     * @param param the default resource creation parameters
     *
     * @note It should be called once at the program startup to set the global default values
     * @note These default values are used when the no-argument version of getInstance() is called
     *       afterwards
     * @note Parameter{} is used if this method is not called
     * @note The maximum number of the resources is determined by the template parameter
     *       MAX_POOL_SIZE_LIMIT (32 by default)
     *
     * @example
     * @code
     * // Initialize it at the program startup
     * Parameter defaultParam;
     * defaultParam.set("host", "localhost");
     * defaultParam.set("port", 3306);
     * ResourceTlsPool<MyResource>::init(defaultParam);
     *
     * // No parameter is needed for the later usage
     * auto& pool = ResourceTlsPool<MyResource>::getInstance();
     * @endcode
     */
    static void init(const Parameter &param) {
        ms_defaultParam = param;
    }

    /**
     * Get the resource pool instance of the current thread (the singleton pattern)
     *
     * @return the resource pool reference of the current thread
     *
     * @note thread_local guarantees that every thread has an independent resource pool instance
     * @note The instance is created with the default parameters set by init(), the maximum pool
     * size is determined by the template parameter MAX_POOL_SIZE_LIMIT
     * @note The instance is created at the first call and the same instance is returned afterwards
     *
     * @example
     * @code
     * // Initialize the default parameters first
     * ResourceTlsPool<MyResource>::init(param);
     *
     * // Get it directly afterwards without a parameter
     * auto& pool = ResourceTlsPool<MyResource>::getInstance();
     * @endcode
     */
    static ResourceTlsPool &getInstance() {
        thread_local static ResourceTlsPool instance(ms_defaultParam);
        return instance;
    }

    /**
     * Destructor, it releases all the cached resources of the current thread
     */
    virtual ~ResourceTlsPool() {
        for (size_t i = 0; i < m_freeCount; ++i) {
            if (m_resourceList[i]) {
                delete m_resourceList[i];
                m_resourceList[i] = nullptr;
            }
        }
        m_freeCount = 0;
    }

    /** The resource deleter, used by shared_ptr to return the resource automatically */
    struct ResourceDeleter {
        ResourceTlsPool *pool;
        std::thread::id owner_thread_id;  // Records the thread ID when the resource is acquired

        void operator()(ResourceType *resource) const {
            if (!resource) {
                return;
            }

            auto current_thread = std::this_thread::get_id();
            if (current_thread != owner_thread_id) {
                // A cross-thread return: the resource is deleted directly to avoid accessing the
                // thread_local variables
                HKU_WARN(
                  "Resource returned from different thread, deleting directly to avoid "
                  "undefined behavior");
                delete resource;
            } else if (pool) {
                // A same-thread return: it is returned to the pool normally
                pool->returnResource(resource);
            } else {
                // The pool is empty (possibly during the destruction), delete it directly
                delete resource;
            }
        }
    };

    /** The resource instance pointer type (the lifetime is managed with shared_ptr) */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * Get an available resource (a synchronous operation)
     *
     * @return the expected object of ResourcePtr, it contains the resource pointer on success and
     *         the error information on failure
     *
     * @note It is a completely lock free operation with an extremely high performance (an O(1) time
     *       complexity)
     * @note The returned shared_ptr returns the resource to the pool automatically at its
     * destruction (through the custom deleter)
     * @note If the current number of the resources has reached the upper limit and there is no idle
     *       resource, the error information is returned
     * @note This is a synchronous method, it returns immediately when called within the same
     * thread, without waiting
     * @note The resource is preferentially got from the idle list of the Ring Buffer; a new one is
     *       created if there is none
     *
     * @par Execution flow
     * 1. Check whether the Ring Buffer has an idle resource (m_freeCount > 0)
     *    - Yes: dequeue it from the head and return the resource pointer
     * 2. If there is no idle resource, check whether the maximum number of the resources is reached
     *    (m_count >= m_maxCount)
     *    - Yes: return an error
     * 3. Create a new resource and increase the count
     */
    stdx::expected<ResourcePtr, std::string> get() {
        // 1. Try to get it from the idle list (dequeue from the head of the Ring Buffer)
        if (m_freeCount > 0) {
            ResourceType *p = m_resourceList[m_head];
            m_resourceList[m_head] = nullptr;
            m_head = (m_head + 1) % MAX_POOL_SIZE_LIMIT;
            m_freeCount--;
            return ResourcePtr(p, ResourceDeleter{this, std::this_thread::get_id()});
        }

        // 2. There is no idle resource, check whether a new one can be created
        if (m_count >= m_maxCount) {
            return stdx::unexpected("No available resources and maximum pool size reached");
        }

        // 3. Create a new resource
        ResourceType *p = nullptr;
        try {
            p = new ResourceType(m_param);
        } catch (const std::exception &e) {
            return stdx::unexpected(fmt::format("Failed to create resource: {}", e.what()));
        } catch (...) {
            return stdx::unexpected("Failed to create resource: Unknown error");
        }

        m_count++;
        return ResourcePtr(p, ResourceDeleter{this, std::this_thread::get_id()});
    }

    /**
     * Create an independently managed resource (not included in the resource pool)
     *
     * @return the expected object of the resource pointer, it contains the resource pointer on
     *         success and the error information on failure
     *
     * @note The returned resource is fully managed by the caller and is not returned to the
     * resource pool
     * @note The resource is deleted directly at its destruction instead of being returned to the
     * pool
     * @note The resource is created with the built-in parameters of the resource pool
     * @note It is suitable for the scenarios needing a temporary resource or a special
     * configuration
     *
     * @example
     * @code
     * auto result = pool.createStandalone();
     * if (result) {
     *     auto res = std::move(result.value());
     *     res->doWork();
     * }  // It is deleted automatically when leaving the scope
     * @endcode
     */
    stdx::expected<std::unique_ptr<ResourceType>, std::string> createStandalone() {
        ResourceType *p = nullptr;
        try {
            p = new ResourceType(m_param);
        } catch (const std::exception &e) {
            return stdx::unexpected(
              fmt::format("Failed to create standalone resource: {}", e.what()));
        } catch (...) {
            return stdx::unexpected("Failed to create standalone resource: Unknown error");
        }

        // Return a standard unique_ptr with the default deleter (a direct delete)
        return std::unique_ptr<ResourceType>(p);
    }

    /** The number of the currently active resources (including the idle and the used ones) */
    size_t count() const {
        return m_count;
    }

    /** The current number of the idle resources */
    size_t idleCount() const {
        return m_freeCount;
    }

    /** Get the maximum number of the resources allowed (the logical upper limit) */
    size_t maxCount() const {
        return m_maxCount;
    }

    /** Set the maximum number of the resources (the logical upper limit, it cannot exceed the
     *  physical capacity) */
    void maxCount(size_t num) {
        if (num > MAX_POOL_SIZE_LIMIT) {
            HKU_WARN("maxCount({}) exceeds physical limit ({}), truncated to {}", num,
                     MAX_POOL_SIZE_LIMIT, MAX_POOL_SIZE_LIMIT);
            m_maxCount = MAX_POOL_SIZE_LIMIT;
        } else {
            m_maxCount = num;
        }
    }

    /**
     * Release all the currently idle resources
     *
     * @note It traverses all the idle resources in the Ring Buffer and deletes them, subtracting
     *       m_freeCount from m_count
     * @note This operation releases all the unused resources immediately, but it does not affect
     * the resources in use
     * @note It is suitable for the scenarios needing to actively reclaim the memory (such as the
     *       idle period of the program)
     * @note The time complexity is O(m_freeCount), all the idle resources need to be traversed
     */
    void releaseIdleResource() {
        for (size_t i = 0; i < m_freeCount; ++i) {
            size_t idx = (m_head + i) % MAX_POOL_SIZE_LIMIT;
            if (m_resourceList[idx]) {
                delete m_resourceList[idx];
                m_resourceList[idx] = nullptr;
            }
        }
        m_count -= m_freeCount;
        m_freeCount = 0;
    }

private:
    ResourceTlsPool(const ResourceTlsPool &) = delete;
    ResourceTlsPool &operator=(const ResourceTlsPool &) = delete;

    /**
     * Constructor (private, accessible through getInstance() only)
     *
     * @param param the resource creation parameters
     */
    explicit ResourceTlsPool(const Parameter &param) : m_param(param) {
        m_resourceList.fill(nullptr);
    }

    /**
     * Default constructor (private, accessible through getInstance() only)
     */
    ResourceTlsPool() {
        m_resourceList.fill(nullptr);
    }

private:
    /**
     * Return the resource to the pool (enqueue at the tail of the Ring Buffer)
     *
     * @param p the resource pointer to be returned
     *
     * @note This method is called automatically by ResourceDeleter, the user does not need to call
     * it manually
     * @note Ring Buffer implementation: the resource pointer is put at the tail position and then
     * the tail index is moved
     * @note If the Ring Buffer is full (m_freeCount == MAX_POOL_SIZE_LIMIT), the resource is
     * deleted directly
     * @note The time complexity is O(1), it is a lock free operation
     *
     * @par The working principle of the Ring Buffer
     * - A fixed size array m_resourceList[MAX_POOL_SIZE_LIMIT] is used
     * - m_head points to the next dequeue position (the resource is taken from here when acquired)
     * - m_tail points to the next enqueue position (the resource is put here when returned)
     * - The circulation is implemented with the modulo operation:
     *   index = (index + 1) % MAX_POOL_SIZE_LIMIT
     * - When head == tail and m_freeCount == 0, it means an empty queue
     * - When m_freeCount == MAX_POOL_SIZE_LIMIT, it means a full queue
     */
    void returnResource(ResourceType *p) {
        if (p && m_freeCount < MAX_POOL_SIZE_LIMIT) {
            m_resourceList[m_tail] = p;
            m_tail = (m_tail + 1) % MAX_POOL_SIZE_LIMIT;
            m_freeCount++;
        } else if (p) {
            // If the Ring Buffer is full, delete the resource directly
            delete p;
        }
    }

private:
    size_t m_maxCount = MAX_POOL_SIZE_LIMIT;  // The logical resource upper limit (configurable at
                                              // runtime, it cannot exceed the physical capacity)
    size_t m_count = 0;                       // The number of the currently active resources
                                              // (including the idle and the used ones)
    size_t m_freeCount = 0;                   // The number of the idle resources
    size_t m_head = 0;  // The head index of the Ring Buffer (the dequeue position)
    size_t m_tail = 0;  // The tail index of the Ring Buffer (the enqueue position)
    Parameter m_param;  // Resource creation parameters
    std::array<ResourceType *, MAX_POOL_SIZE_LIMIT>
      m_resourceList{};  // The idle resource array (the Ring Buffer, the physical capacity is
                         // fixed)

private:
    static Parameter ms_defaultParam;
};

// Static member initialization
template <typename ResourceType, size_t MAX_POOL_SIZE_LIMIT>
Parameter ResourceTlsPool<ResourceType, MAX_POOL_SIZE_LIMIT>::ms_defaultParam{};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_TLS_POOL_H */