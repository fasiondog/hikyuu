#pragma once
#ifndef HKU_UTILS_RESOURCE_TLS_VERSION_POOL_H
#define HKU_UTILS_RESOURCE_TLS_VERSION_POOL_H

#include <array>
#include <memory>
#include <string>
#include "expected.h"
#include "Parameter.h"
#include "Log.h"
#include "ResourceVersionTraits.h"

/**
 * Thread local versioned resource pool - a lock free design supporting the lightweight version
 * management (the Ring Buffer implementation)
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
 *          - Version management: the parameter change detection is supported and the old version
 *          resources are destroyed automatically
 *
 *          **Important limitations**:
 *          - It is not suitable for the coroutine environment: a coroutine may be migrated between
 *          different threads, causing the resource to be returned across the threads
 *          - On a cross-thread return the resource is deleted directly and a warning log is output,
 *          so the resource reuse cannot be achieved
 *          - If a resource pool is needed in the coroutine environment, it is recommended to use
 * the global shared pool (such as ResourceAsioVersionPool)
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
 *          The main differences from ResourceTlsPool:
 *          - The version number management is supported, so the parameter change can be detected
 *          - The version is checked when the resource is returned, and the old version resources
 * are destroyed automatically
 *          - The version is checked when the resource is acquired; a new version resource is
 * created if it does not match
 *          - The global version and the parameters are synchronized through the syncVersion()
 * method
 *
 * @brief TLS versioned resource pool (the thread local storage + the version number management)
 * @details Every thread has an independent resource pool instance.
 *          When the parameters change, the version number is increased automatically and all the
 * idle old version resources are released.
 *
 *          **Important constraint**: ResourceType must implement the getVersion() and
 *          setVersion(int) methods.
 *
 * @tparam ResourceType the resource type, it must implement the getVersion() and setVersion(int)
 *                      methods
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
 * ResourceTlsVersionPool<MyResource>::init(param);
 *
 * // Step 2: get the thread local resource pool instance
 * auto& pool = ResourceTlsVersionPool<MyResource>::getInstance();
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
 * @note The version number is thread local, it is not synchronized across the threads
 * @warning It is not recommended to use it in the coroutine environment; the thread migration of
 * the coroutine would make the resource reuse impossible
 */
namespace hku {

template <typename ResourceType, size_t MAX_POOL_SIZE_LIMIT = 2>
class ResourceTlsVersionPool {
public:
    // Compile-time check: ResourceType must support getVersion and setVersion
    static_assert(detail::has_resource_getVersion_v<ResourceType>,
                  "ResourceType must implement getVersion() method.");
    static_assert(detail::has_resource_setVersion_v<ResourceType>,
                  "ResourceType must implement setVersion(int) method.");

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
     * ResourceTlsVersionPool<MyResource>::init(defaultParam);
     *
     * // No parameter is needed for the later usage
     * auto& pool = ResourceTlsVersionPool<MyResource>::getInstance();
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
     * ResourceTlsVersionPool<MyResource>::init(param);
     *
     * // Get it directly afterwards without a parameter
     * auto& pool = ResourceTlsVersionPool<MyResource>::getInstance();
     * @endcode
     */
    static ResourceTlsVersionPool &getInstance() {
        thread_local static ResourceTlsVersionPool instance(ms_defaultParam);
        return instance;
    }

    /**
     * Destructor, it releases all the cached resources of the current thread
     */
    virtual ~ResourceTlsVersionPool() {
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
        ResourceTlsVersionPool *pool;
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
     * Synchronously update the version and the parameters (called by HybridVersionPool)
     *
     * @param version the new version number
     * @param param the new parameters
     *
     * @note This method releases all the idle old version resources
     * @note It only affects the resource pool instance of the current thread
     */
    void syncVersion(int version, const Parameter &param) {
        if (version == m_version) {
            return;  // The version is the same, no update is needed
        }

        // Release all the idle old version resources
        releaseIdleResource();

        // Update the version and the parameters
        m_version = version;
        m_param = param;
    }

    /**
     * Get the current version number
     *
     * @return the version number of the current thread
     */
    int getVersion() const {
        return m_version;
    }

    /**
     * Get an available resource
     *
     * @return the expected object of ResourcePtr, it contains the resource pointer on success and
     *         the error information on failure
     *
     * @note It is a completely lock free operation with an extremely high performance
     * @note The returned shared_ptr returns the resource to the pool automatically at its
     * destruction (through the custom deleter)
     * @note If the current number of the resources has reached the upper limit and there is no idle
     *       resource, the error information is returned
     * @note The version is checked when it is acquired from the idle queue; when the version does
     * not match, the old resource is destroyed and a new resource is created
     */
    stdx::expected<ResourcePtr, std::string> get() {
        // 1. Try to get it from the idle list (dequeue from the head of the Ring Buffer)
        if (m_freeCount > 0) {
            ResourceType *p = m_resourceList[m_head];
            m_resourceList[m_head] = nullptr;
            m_head = (m_head + 1) % MAX_POOL_SIZE_LIMIT;
            m_freeCount--;

            // Check whether the resource version matches
            if (p->getVersion() != m_version) {
                // The version does not match, destroy the old resource
                delete p;
                m_count--;

                // Try to create a new version resource
                if (m_count < m_maxCount) {
                    try {
                        p = new ResourceType(m_param);
                        p->setVersion(m_version);
                        m_count++;
                        return ResourcePtr(p, ResourceDeleter{this, std::this_thread::get_id()});
                    } catch (const std::exception &e) {
                        return stdx::unexpected(
                          fmt::format("Failed to create resource: {}", e.what()));
                    } catch (...) {
                        return stdx::unexpected("Failed to create resource: Unknown error");
                    }
                } else {
                    return stdx::unexpected("No available resources and maximum pool size reached");
                }
            }

            // The version matches, return normally
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
            p->setVersion(m_version);
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
            p->setVersion(m_version);
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

    /** Release all the currently idle resources */
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
    ResourceTlsVersionPool(const ResourceTlsVersionPool &) = delete;
    ResourceTlsVersionPool &operator=(const ResourceTlsVersionPool &) = delete;

    /**
     * Constructor (private, accessible through getInstance() only)
     *
     * @param param the resource creation parameters
     */
    explicit ResourceTlsVersionPool(const Parameter &param) : m_param(param), m_version(0) {
        m_resourceList.fill(nullptr);
    }

    /**
     * Default constructor (private, accessible through getInstance() only)
     */
    ResourceTlsVersionPool() : m_version(0) {
        m_resourceList.fill(nullptr);
    }

private:
    /** Return the resource to the pool (enqueue at the tail of the Ring Buffer) */
    void returnResource(ResourceType *p) {
        if (!p) {
            return;
        }

        // Check whether the resource version matches
        if (p->getVersion() != m_version) {
            // The version does not match, destroy it directly
            delete p;
            m_count--;
            return;
        }

        // The version matches, return it to the pool
        if (m_freeCount < MAX_POOL_SIZE_LIMIT) {
            m_resourceList[m_tail] = p;
            m_tail = (m_tail + 1) % MAX_POOL_SIZE_LIMIT;
            m_freeCount++;
        } else {
            // If the Ring Buffer is full, delete the resource directly
            delete p;
            m_count--;
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
    int m_version = 0;  // The version number of the current thread
    std::array<ResourceType *, MAX_POOL_SIZE_LIMIT>
      m_resourceList{};  // The idle resource array (the Ring Buffer, the physical capacity is
                         // fixed)

private:
    static Parameter ms_defaultParam;
};

// Static member initialization
template <typename ResourceType, size_t MAX_POOL_SIZE_LIMIT>
Parameter ResourceTlsVersionPool<ResourceType, MAX_POOL_SIZE_LIMIT>::ms_defaultParam{};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_TLS_VERSION_POOL_H */