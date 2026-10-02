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
#include <deque>
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
 * General shared resource pool - it is suitable for the coroutine environment
 * It uses the boost lock free queue and acquires the resources asynchronously in a coroutine
 * @ingroup Utilities
 *
 * @tparam ResourceType the resource type, it must support the constructor
 * ResourceType(const Parameter&)
 * @tparam MutexType the mutex type, std::mutex by default (thread safe)
 */
template <typename ResourceType, typename MutexType = std::mutex>
class ResourceAsioPool {
private:
    /** The node of one waiting coroutine
     *  It is co-owned by the waiting queue and by the waiting coroutine (shared_ptr), so neither
     *  side can ever observe a node that the other one has already destroyed. The ownership of the
     *  resource handed over is decided by exactly one atomic operation on `handed`:
     *    - nullptr      nobody has claimed it yet, a resource may still be handed over
     *    - a resource   a returning thread has handed its resource over; whoever reads this value
     *                   owns that resource
     *    - gaveUp()     somebody has taken the value over (the waiting coroutine itself, or the
     *                   destructor), nobody may hand anything over any more
     */
    struct WaiterNode {
        std::shared_ptr<net::steady_timer> timer;
        std::atomic<ResourceType *> handed{nullptr};

        /** The "nothing more to hand over" marker, it is never dereferenced */
        static ResourceType *gaveUp() noexcept {
            return reinterpret_cast<ResourceType *>(std::uintptr_t(1));
        }
    };

    /** The waiting queue, shared with the waiting coroutines: a coroutine that resumes after the
     *  pool is gone unregisters itself through its own reference instead of touching the pool.
     *  Only weak references are stored, the waiting coroutine itself holds the only strong one, so
     *  a coroutine destroyed while suspended leaves nothing behind in the queue. The capacity is
     *  reserved up front, so joining the queue never allocates and never throws */
    struct WaiterRegistry {
        std::mutex mutex;
        std::vector<std::weak_ptr<WaiterNode>> queue;
    };

public:
    ResourceAsioPool() = delete;
    ResourceAsioPool(const ResourceAsioPool &) = delete;
    ResourceAsioPool &operator=(const ResourceAsioPool &) = delete;

    /**
     * Constructor
     * @param param connection parameters
     * @param max_count the maximum resource upper limit, 0 means unlimited
     */
    explicit ResourceAsioPool(const Parameter &param, size_t max_count = 128,
                              size_t max_waiters = 1000)
    : m_maxCount(max_count),
      m_param(param),
      m_resourceList(max_count == 0 ? 128 : max_count),
      m_maxWaiters(max_waiters > 0 ? max_waiters : 1000),
      m_waiters(std::make_shared<WaiterRegistry>()) {
        // The waiting queue is reserved up front, so joining it never allocates and never throws
        m_waiters->queue.reserve(m_maxWaiters);
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
     * Destructor, it releases all the cached resources
     */
    virtual ~ResourceAsioPool() {
        // Mark that it is being destructed, preventing new resource acquisitions and returns
        m_is_destroying.store(true, std::memory_order_release);

        // Take all the waiters away at once. Every node is co-owned by the waiting queue and by the
        // waiting coroutine, so whoever claims the reserved resource first owns it, while the node
        // itself stays alive as long as the other side still holds a reference.
        std::vector<std::weak_ptr<WaiterNode>> waiters;
        {
            std::lock_guard<std::mutex> lock(m_waiters->mutex);
            waiters.swap(m_waiters->queue);
        }
        m_registeredWaiters.fetch_sub(waiters.size(), std::memory_order_relaxed);

        for (const auto &waiter_ref : waiters) {
            auto waiter = waiter_ref.lock();
            if (!waiter) {
                // The waiting coroutine is gone, its node dies together with it
                continue;
            }
            ResourceType *p =
              waiter->handed.exchange(WaiterNode::gaveUp(), std::memory_order_acq_rel);
            // Only the resource nobody will ever claim may be released here. The waiting coroutines
            // are not timed out explicitly: whenever one of them resumes it finds gaveUp() and
            // takes the time out branch, which reads nothing from the pool any more.
            if (p != nullptr && p != WaiterNode::gaveUp()) {
                delete p;
                releaseResourceCount();
            }
        }

        // Wait for all the active resources to be returned
        // When m_count == m_idleCount all the resources have been returned to the idle queue
        std::unique_lock<MutexType> lock(m_destroy_mutex);
        m_destroy_waiting.store(true, std::memory_order_seq_cst);
        m_destroy_cv.wait(lock, [this]() {
            return m_count.load(std::memory_order_seq_cst) ==
                   m_idleCount.load(std::memory_order_seq_cst);
        });
        m_destroy_waiting.store(false, std::memory_order_relaxed);

        // At this time all the resources are in the idle queue, release them
        ResourceType *p = nullptr;
        while (m_resourceList.pop(p)) {
            if (p) {
                delete p;
            }
        }
    }

    /** Resource instance pointer type */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * Acquire an available resource synchronously (without waiting; it returns a failure directly
     * when there is no idle resource)
     *
     * @return stdx::expected<ResourcePtr, std::string>
     *         it contains the resource pointer on success and the error information on failure
     *
     * @note The acquisition strategy:
     *       1. Acquire the resource from the idle queue preferentially and check its version
     *       2. If the resource version is too old, destroy the resource and try to create a new one
     *       3. If there is no idle resource but the upper limit has not been reached, create a new
     *          version resource
     *       4. If the upper limit has been reached and there is no idle resource, return a failure
     *          immediately (without waiting)
     *       5. Use asyncGet() if an asynchronous waiting is needed
     *
     * @example
     * @code
     * auto result = pool.get();
     * if (result) {
     *     auto resource = result.value();
     *     // The resource is guaranteed to be of the current latest version
     *     resource->doWork();
     * } else {
     *     HKU_ERROR("Failed to get resource: {}", result.error());
     * }
     * @endcode
     */
    stdx::expected<ResourcePtr, std::string> get() {
        // 1. Try to acquire a resource from the idle queue
        ResourceType *p = takeIdleResource();
        if (p) {
            return stdx::expected<ResourcePtr, std::string>(ResourcePtr(p, ResourceCloser(this)));
        }

        // 2. There is no idle resource but the upper limit has not been reached -> create a new
        //    resource. The slot is reserved with a CAS first, so the upper limit stays a hard one
        //    even when several threads create resources at the same time
        if (reserveResourceSlot()) {
            try {
                p = new ResourceType(m_param);
            } catch (const std::exception &e) {
                releaseResourceCount();
                return stdx::unexpected(
                  std::string(fmt::format("Failed create a new Resource! {}", e.what())));
            } catch (...) {
                releaseResourceCount();
                return stdx::unexpected(
                  std::string("Failed create a new Resource! Unknown error!"));
            }
            return stdx::expected<ResourcePtr, std::string>(ResourcePtr(p, ResourceCloser(this)));
        }

        // 3. The upper limit has been reached and there is no idle resource -> return a failure
        //    directly
        return stdx::unexpected(fmt::format("No available resource, max_count={}, current_count={}",
                                            m_maxCount, m_count.load(std::memory_order_relaxed)));
    }

    /**
     * Acquire an available resource in a coroutine way (with a timeout)
     * @param timeout timeout
     * @return awaitable<stdx::expected<ResourcePtr, std::string>>
     * an awaitable result, it contains the resource pointer on success and the error information on
     * failure
     */
    awaitable<stdx::expected<ResourcePtr, std::string>> asyncGet(
      std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
        // Try to acquire a resource from the idle queue
        ResourceType *p = takeIdleResource();
        if (p) {
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(p, ResourceCloser(this)));
        }

        // There is no idle resource but the upper limit has not been reached -> create a new
        // resource. The slot is reserved with a CAS first, so the upper limit stays a hard one even
        // when several coroutines create resources at the same time
        if (reserveResourceSlot()) {
            try {
                p = new ResourceType(m_param);
            } catch (const std::exception &e) {
                releaseResourceCount();
                co_return stdx::unexpected(
                  std::string(fmt::format("Failed create a new Resource! {}", e.what())));
            } catch (...) {
                releaseResourceCount();
                co_return stdx::unexpected(
                  std::string("Failed create a new Resource! Unknown error!"));
            }
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(p, ResourceCloser(this)));
        }

        // The upper limit has been reached -> enter the waiting queue
        auto executor = co_await this_coro::executor;
        auto waiter = std::make_shared<WaiterNode>();
        waiter->timer = std::make_shared<net::steady_timer>(executor);
        waiter->timer->expires_after(timeout);

        // Both messages are built before the wait is started: nothing may be read from the pool on
        // the failure branches, because the pool can already be gone when this coroutine resumes,
        // and nothing between the wait being started and its co_await may throw (an awaitable with
        // a pending operation must never be destroyed without being awaited)
        auto timeout_msg =
          fmt::format("ResourceAsioPool get timeout, max_count={}, current_count={}", m_maxCount,
                      m_count.load(std::memory_order_relaxed));
        auto queue_full_msg = fmt::format("Waiter queue is full (max={})", m_maxWaiters);

        // Start the wait before joining the queue, so that every wake up done by a resource
        // returning thread hits an outstanding operation
        net::error_code ec;
        auto wait_result = waiter->timer->async_wait(net::redirect_error(net::use_awaitable, ec));

        // Keep our own reference to the waiting queue, so that unregistering stays valid even when
        // the pool is destroyed while we are suspended
        auto waiters = m_waiters;

        if (!pushWaiter(waiters, waiter)) {
            waiter->timer->expires_after(std::chrono::milliseconds(0));
            co_await std::move(wait_result);
            co_return stdx::unexpected(queue_full_msg);
        }

        // A resource may have been handed over between the checks above and joining the queue, take
        // it instead of sleeping for a whole timeout
        ResourceType *idle = takeIdleResource();
        if (idle) {
            waiter->timer->expires_after(std::chrono::milliseconds(0));
            co_await std::move(wait_result);
            ResourceType *reserved =
              waiter->handed.exchange(WaiterNode::gaveUp(), std::memory_order_acq_rel);
            removeWaiter(waiters, waiter);
            if (reserved != nullptr && reserved != WaiterNode::gaveUp()) {
                // Somebody handed a second resource over to us, give ours back
                returnResource(idle, nullptr);
                idle = reserved;
            }
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(idle, ResourceCloser(this)));
        }

        // Note the error code is not looked at: it only tells how we were woken up, the result is
        // decided by who wins the atomic exchange below
        co_await std::move(wait_result);

        ResourceType *reserved =
          waiter->handed.exchange(WaiterNode::gaveUp(), std::memory_order_acq_rel);
        removeWaiter(waiters, waiter);

        if (reserved != nullptr && reserved != WaiterNode::gaveUp()) {
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(reserved, ResourceCloser(this)));
        }

        co_return stdx::unexpected(timeout_msg);
    }

    /** The number of the currently active resources, i.e. all the resources (including the idle and
     *  the used ones) */
    size_t count() const {
        return m_count.load();
    }

    /**
     * The current number of the idle resources (the exact value)
     * It is tracked with an atomic counter to avoid operating the queue itself
     */
    size_t idleCount() const {
        return m_idleCount.load();
    }

    /** Release all the currently idle resources */
    void releaseIdleResource() {
        ResourceType *p = nullptr;
        while (m_resourceList.pop(p)) {
            if (p) {
                m_idleCount.fetch_sub(1);  // Decrease the idle count
                delete p;
                releaseResourceCount();  // Decrease the count and notify the destructor
            }
        }
    }

private:
    /** Pop one resource from the idle queue, nullptr when there is none */
    ResourceType *takeIdleResource() {
        ResourceType *p = nullptr;
        if (m_resourceList.pop(p)) {
            m_idleCount.fetch_sub(1, std::memory_order_relaxed);
            return p;
        }
        return nullptr;
    }

    /** Decrease the active resource count and notify the destructor waiting on it. The decrement
     *  may turn the destructor predicate true, so it is ordered against the waiting flag */
    void releaseResourceCount() {
        m_count.fetch_sub(1, std::memory_order_seq_cst);
        notifyCountChanged();
    }

    /** Notify the destructor that the resource counters have changed. The destructor is the only
     *  waiter on the condition variable, so while it is not waiting the notification is skipped to
     *  keep the resource returning path free of locks. The seq_cst order on both the flag and the
     *  counter operations closes the classic lost wakeup gap: either the notifier sees the flag
     *  (and then notifies after taking the mutex, which serializes with the waiting destructor) or
     *  the destructor sees the counter change when it evaluates its predicate */
    void notifyCountChanged() {
        if (m_destroy_waiting.load(std::memory_order_seq_cst)) {
            std::unique_lock<MutexType> lock(m_destroy_mutex);
            m_destroy_cv.notify_one();
        }
    }

    /** Join the waiting queue, false when it is full */
    bool pushWaiter(const std::shared_ptr<WaiterRegistry> &waiters,
                    const std::shared_ptr<WaiterNode> &waiter) {
        std::lock_guard<std::mutex> lock(waiters->mutex);
        if (waiters->queue.size() >= m_maxWaiters) {
            return false;
        }
        waiters->queue.emplace_back(waiter);
        m_registeredWaiters.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    /** Unregister our own waiter node, it is a no-op when somebody else already took it */
    void removeWaiter(const std::shared_ptr<WaiterRegistry> &waiters,
                      const std::shared_ptr<WaiterNode> &waiter) {
        std::lock_guard<std::mutex> lock(waiters->mutex);
        for (auto iter = waiters->queue.begin(); iter != waiters->queue.end(); ++iter) {
            if (iter->lock() == waiter) {
                waiters->queue.erase(iter);
                m_registeredWaiters.fetch_sub(1, std::memory_order_relaxed);
                return;
            }
        }
    }

    /** Hand a resource over to the first waiter that has not given up yet, true on success: the
     *  waiter owns the resource from then on, no matter what happens to the node afterwards.
     *  The node stays in the queue until its coroutine unregisters itself, so that the destructor
     *  can still take back a resource that nobody will claim */
    bool handToWaiter(ResourceType *p) {
        // The fast path of the resource returning: without any registered waiter there is nothing
        // to scan and no lock to take. A waiter registered just after this check finds the resource
        // by itself through the idle queue re-check below
        if (m_registeredWaiters.load(std::memory_order_acquire) == 0) {
            return false;
        }

        while (true) {
            std::shared_ptr<WaiterNode> waiter;
            {
                std::lock_guard<std::mutex> lock(m_waiters->mutex);
                for (auto iter = m_waiters->queue.begin(); iter != m_waiters->queue.end();) {
                    auto candidate = iter->lock();
                    if (!candidate) {
                        // The waiting coroutine is gone, clean its node up on the way
                        iter = m_waiters->queue.erase(iter);
                        m_registeredWaiters.fetch_sub(1, std::memory_order_relaxed);
                        continue;
                    }
                    ResourceType *current = candidate->handed.load(std::memory_order_relaxed);
                    if (current == WaiterNode::gaveUp()) {
                        // It has given up, clean its node up on the way
                        iter = m_waiters->queue.erase(iter);
                        m_registeredWaiters.fetch_sub(1, std::memory_order_relaxed);
                        continue;
                    }
                    if (current == nullptr) {
                        waiter = std::move(candidate);
                        break;
                    }
                    // A resource has been handed over to it but has not been taken yet
                    ++iter;
                }
                if (!waiter) {
                    return false;
                }
            }

            ResourceType *expected = nullptr;
            if (waiter->handed.compare_exchange_strong(expected, p, std::memory_order_acq_rel)) {
                // Wake it up with cancel(): it never touches the expiry, which the waiting
                // coroutine may rearm at the very same moment
                waiter->timer->cancel();
                return true;
            }

            // It has just given up, look for the next one
        }
    }

    /** Reserve a slot for a new resource with a CAS, false when the upper limit has been reached.
     *  Reserving first keeps the upper limit a hard one even when several threads create
     *  resources at the same time */
    bool reserveResourceSlot() {
        size_t current = m_count.load(std::memory_order_relaxed);
        while (m_maxCount == 0 || current < m_maxCount) {
            if (m_count.compare_exchange_weak(current, current + 1, std::memory_order_acq_rel)) {
                return true;
            }
        }
        return false;
    }

    class ResourceCloser {
    public:
        explicit ResourceCloser(ResourceAsioPool *pool) : m_pool(pool) {}

        void operator()(ResourceType *conn) {
            if (conn) {
                // If the pool is bound, the resource is returned; otherwise it is deleted
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

    /** Return it to the resource pool */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        if (!p) [[unlikely]] {
            HKU_WARN("ResourceAsioPool::returnResource: nullptr");
            return;
        }

        // If it is being destructed, delete the resource directly
        if (m_is_destroying.load(std::memory_order_acquire)) {
            delete p;
            releaseResourceCount();
            return;
        }

        // Try to hand it over to a waiter that has not given up yet
        if (handToWaiter(p)) {
            return;
        }

        // There is no waiter to hand it over to, put it back into the idle queue
        if (!m_resourceList.push(p)) {
            // The queue is full, delete it directly
            delete p;
            releaseResourceCount();
            return;
        }

        m_idleCount.fetch_add(1, std::memory_order_seq_cst);
        notifyCountChanged();
    }

    std::atomic<size_t> m_count{0};      // The number of the currently active resources
    std::atomic<size_t> m_idleCount{0};  // The current number of the idle resources
    size_t m_maxCount;                   // The maximum resource upper limit
    Parameter m_param;
    boost::lockfree::queue<ResourceType *> m_resourceList;

    size_t m_maxWaiters;  // The runtime logical upper limit: the maximum number of the waiters
    std::shared_ptr<WaiterRegistry> m_waiters;  // The waiting queue, shared with the waiting
                                                // coroutines
    MutexType m_destroy_mutex;                  // The mutex protecting the destructor waiting
                                                // condition variable
    std::condition_variable_any m_destroy_cv;   // Used to notify the destructor that a
                                                // resource has been returned
    std::atomic<bool> m_is_destroying{false};   // Marks whether it is being destructed
    std::atomic<size_t> m_registeredWaiters{
      0};  // The number of the registered waiters, it may be a little too large
    std::atomic<bool> m_destroy_waiting{
      false};  // Marks whether the destructor is waiting on the condition variable
};

/**
 * @brief Versioned resource pool (the resource type is required to support the version interfaces)
 * @details It uses boost::lockfree::queue to implement the lock free idle queue and supports the
 *          asynchronous resource acquisition in a coroutine.
 *          When the parameters change, the version number is increased automatically and all the
 * idle old version resources are released.
 *
 *          **Important constraint**: ResourceType must implement the getVersion() and
 *          setVersion(int) methods.
 *
 * @tparam ResourceType the resource type, it must implement the getVersion() and setVersion(int)
 *                      methods
 * @tparam MutexType the mutex type, std::mutex by default (thread safe)
 * @ingroup Utilities
 */
template <typename ResourceType, typename MutexType = std::mutex>
class ResourceAsioVersionPool {
private:
    /** The node of one waiting coroutine
     *  It is co-owned by the waiting queue and by the waiting coroutine (shared_ptr), so neither
     *  side can ever observe a node that the other one has already destroyed. The ownership of the
     *  resource handed over is decided by exactly one atomic operation on `handed`:
     *    - nullptr      nobody has claimed it yet, a resource may still be handed over
     *    - a resource   a returning thread has handed its resource over; whoever reads this value
     *                   owns that resource
     *    - gaveUp()     somebody has taken the value over (the waiting coroutine itself, or the
     *                   destructor), nobody may hand anything over any more
     */
    struct WaiterNode {
        std::shared_ptr<net::steady_timer> timer;
        std::atomic<ResourceType *> handed{nullptr};

        /** The "nothing more to hand over" marker, it is never dereferenced */
        static ResourceType *gaveUp() noexcept {
            return reinterpret_cast<ResourceType *>(std::uintptr_t(1));
        }
    };

    /** The waiting queue, shared with the waiting coroutines: a coroutine that resumes after the
     *  pool is gone unregisters itself through its own reference instead of touching the pool.
     *  Only weak references are stored, the waiting coroutine itself holds the only strong one, so
     *  a coroutine destroyed while suspended leaves nothing behind in the queue. The capacity is
     *  reserved up front, so joining the queue never allocates and never throws */
    struct WaiterRegistry {
        std::mutex mutex;
        std::vector<std::weak_ptr<WaiterNode>> queue;
    };

public:
    // Compile-time check: ResourceType must support getVersion and setVersion
    static_assert(hku::detail::has_resource_getVersion_v<ResourceType>,
                  "ResourceType must implement getVersion() method.");
    static_assert(hku::detail::has_resource_setVersion_v<ResourceType>,
                  "ResourceType must implement setVersion(int) method.");

    ResourceAsioVersionPool() = delete;
    ResourceAsioVersionPool(const ResourceAsioVersionPool &) = delete;
    ResourceAsioVersionPool &operator=(const ResourceAsioVersionPool &) = delete;

    /**
     * Constructor
     * @param param connection parameters
     * @param max_count the maximum resource upper limit, 0 means unlimited
     * @param max_waiters the maximum number of the waiters
     */
    explicit ResourceAsioVersionPool(const Parameter &param, size_t max_count = 0,
                                     size_t max_waiters = 1000)
    : m_maxCount(max_count),
      m_param(param),
      m_resourceList(128),
      m_maxWaiters(max_waiters > 0 ? max_waiters : 1000),
      m_waiters(std::make_shared<WaiterRegistry>()) {
        // The waiting queue is reserved up front, so joining it never allocates and never throws
        m_waiters->queue.reserve(m_maxWaiters);
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
     * Destructor, it releases all the cached resources
     */
    virtual ~ResourceAsioVersionPool() {
        // Mark that it is being destructed, preventing new resource acquisitions and returns
        m_is_destroying.store(true, std::memory_order_release);

        // Take all the waiters away at once. Every node is co-owned by the waiting queue and by the
        // waiting coroutine, so whoever claims the reserved resource first owns it, while the node
        // itself stays alive as long as the other side still holds a reference.
        std::vector<std::weak_ptr<WaiterNode>> waiters;
        {
            std::lock_guard<std::mutex> lock(m_waiters->mutex);
            waiters.swap(m_waiters->queue);
        }
        m_registeredWaiters.fetch_sub(waiters.size(), std::memory_order_relaxed);

        for (const auto &waiter_ref : waiters) {
            auto waiter = waiter_ref.lock();
            if (!waiter) {
                // The waiting coroutine is gone, its node dies together with it
                continue;
            }
            ResourceType *p =
              waiter->handed.exchange(WaiterNode::gaveUp(), std::memory_order_acq_rel);
            // Only the resource nobody will ever claim may be released here. The waiting coroutines
            // are not timed out explicitly: whenever one of them resumes it finds gaveUp() and
            // takes the time out branch, which reads nothing from the pool any more.
            if (p != nullptr && p != WaiterNode::gaveUp()) {
                delete p;
                releaseResourceCount();
            }
        }

        // Wait for all the active resources to be returned
        std::unique_lock<MutexType> lock(m_destroy_mutex);
        m_destroy_waiting.store(true, std::memory_order_seq_cst);
        m_destroy_cv.wait(lock, [this]() {
            return m_count.load(std::memory_order_seq_cst) ==
                   m_idleCount.load(std::memory_order_seq_cst);
        });
        m_destroy_waiting.store(false, std::memory_order_relaxed);

        // At this time all the resources are in the idle queue, release them
        ResourceType *p = nullptr;
        while (m_resourceList.pop(p)) {
            if (p) {
                delete p;
            }
        }
    }

    /** Whether the given parameter exists */
    bool haveParam(const std::string &name) {
        std::lock_guard<MutexType> lock(m_mutex);
        return m_param.have(name);
    }

    /** Get the value of the given parameter; an exception is thrown when the parameter does not
     * exist or the type does not match */
    template <typename ValueType>
    ValueType getParam(const std::string &name) {
        std::lock_guard<MutexType> lock(m_mutex);
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
        std::lock_guard<MutexType> lock(m_mutex);
        // If the parameter has not actually changed, return directly
        if (m_param.have(name) && value == m_param.get<ValueType>(name)) {
            return;
        }
        m_param.set<ValueType>(name, value);
        m_version.fetch_add(1);
        releaseIdleResource();  // Release the current idle resources so that the new parameter
                                // values take effect
    }

    /**
     * @brief Set the resource parameters; they take effect only when a new resource is created
     * @param param the parameter object
     */
    void setParameter(const Parameter &param) {
        std::lock_guard<MutexType> lock(m_mutex);
        m_param = param;
        m_version.fetch_add(1);
        releaseIdleResource();  // Release the current idle resources so that the new parameter
                                // values take effect
    }

    /**
     * @brief Set the resource parameters; they take effect only when a new resource is created
     * @param param the parameter object
     */
    void setParameter(Parameter &&param) {
        std::lock_guard<MutexType> lock(m_mutex);
        m_param = std::move(param);
        m_version.fetch_add(1);
        releaseIdleResource();  // Release the current idle resources so that the new parameter
                                // values take effect
    }

    /** Get the current version of the resource pool */
    int getVersion() {
        return m_version.load();
    }

    /** Increase the current version of the resource pool, equivalent to notifying the resource pool
     *  that the resource version has changed */
    void incVersion(int version) {
        m_version.fetch_add(1);
    }

    /** Resource instance pointer type */
    typedef std::shared_ptr<ResourceType> ResourcePtr;

    /**
     * Acquire an available resource synchronously (without waiting; it returns a failure directly
     * when there is no idle resource)
     * @return stdx::expected<ResourcePtr, std::string>
     * It contains the resource pointer on success and the error information on failure
     */
    stdx::expected<ResourcePtr, std::string> get() {
        // 1. Try to acquire a resource from the idle queue; a resource whose version is too old is
        //    destroyed by takeIdleResource() itself and nullptr is returned then
        ResourceType *p = takeIdleResource();
        if (p) {
            return stdx::expected<ResourcePtr, std::string>(ResourcePtr(p, ResourceCloser(this)));
        }

        // 2. The upper limit has not been reached, create a new resource. The slot is reserved with
        //    a CAS first, so the upper limit stays a hard one even when several threads create
        //    resources at the same time
        if (reserveResourceSlot()) {
            try {
                Parameter current_param;
                {
                    std::lock_guard<MutexType> lock(m_mutex);
                    current_param = m_param;
                }

                p = new ResourceType(current_param);
                p->setVersion(m_version.load());
            } catch (const std::exception &e) {
                releaseResourceCount();
                return stdx::unexpected(
                  std::string(fmt::format("Failed create a new Resource! {}", e.what())));
            } catch (...) {
                releaseResourceCount();
                return stdx::unexpected(
                  std::string("Failed create a new Resource! Unknown error!"));
            }

            return stdx::expected<ResourcePtr, std::string>(ResourcePtr(p, ResourceCloser(this)));
        }

        // 3. The upper limit has been reached and there is no idle resource -> return a failure
        //    directly
        return stdx::unexpected(fmt::format("No available resource, max_count={}, current_count={}",
                                            m_maxCount, m_count.load(std::memory_order_relaxed)));
    }

    /**
     * Acquire an available resource in a coroutine way (with a timeout)
     * @param timeout timeout, 5 seconds by default
     * @return awaitable<stdx::expected<ResourcePtr, std::string>>
     * an awaitable result, it contains the resource pointer on success and the error information on
     * failure
     */
    awaitable<stdx::expected<ResourcePtr, std::string>> asyncGet(
      std::chrono::steady_clock::duration timeout = std::chrono::seconds(5)) {
        auto executor = co_await this_coro::executor;

        // Try to acquire a resource from the idle queue; a resource whose version is too old is
        // destroyed by takeIdleResource() itself and nullptr is returned then, a new resource is
        // created below
        ResourceType *p = takeIdleResource();
        if (p) {
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(p, ResourceCloser(this)));
        }

        // The upper limit has not been reached, create a new resource. The slot is reserved with a
        // CAS first, so the upper limit stays a hard one even when several coroutines create
        // resources at the same time
        if (reserveResourceSlot()) {
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
                releaseResourceCount();
                co_return stdx::unexpected(
                  std::string(fmt::format("Failed create a new Resource! {}", e.what())));
            } catch (...) {
                releaseResourceCount();
                co_return stdx::unexpected(
                  std::string("Failed create a new Resource! Unknown error!"));
            }

            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(p, ResourceCloser(this)));
        }

        // The upper limit has been reached, enter the waiting queue
        auto waiter = std::make_shared<WaiterNode>();
        waiter->timer = std::make_shared<net::steady_timer>(executor);
        waiter->timer->expires_after(timeout);

        // Both messages are built before the wait is started: nothing may be read from the pool on
        // the failure branches, because the pool can already be gone when this coroutine resumes,
        // and nothing between the wait being started and its co_await may throw (an awaitable with
        // a pending operation must never be destroyed without being awaited)
        auto timeout_msg =
          fmt::format("ResourceAsioVersionPool get timeout, max_count={}, current_count={}",
                      m_maxCount, m_count.load(std::memory_order_relaxed));
        auto queue_full_msg = fmt::format("Waiter queue is full (max={})", m_maxWaiters);

        // Start the wait before joining the queue, so that every wake up done by a resource
        // returning thread hits an outstanding operation
        net::error_code ec;
        auto wait_result = waiter->timer->async_wait(net::redirect_error(net::use_awaitable, ec));

        // Keep our own reference to the waiting queue, so that unregistering stays valid even when
        // the pool is destroyed while we are suspended
        auto waiters = m_waiters;

        if (!pushWaiter(waiters, waiter)) {
            waiter->timer->expires_after(std::chrono::milliseconds(0));
            co_await std::move(wait_result);
            co_return stdx::unexpected(queue_full_msg);
        }

        // A resource may have been handed over between the checks above and joining the queue, take
        // it instead of sleeping for a whole timeout
        ResourceType *idle = takeIdleResource();
        if (idle) {
            waiter->timer->expires_after(std::chrono::milliseconds(0));
            co_await std::move(wait_result);
            ResourceType *reserved =
              waiter->handed.exchange(WaiterNode::gaveUp(), std::memory_order_acq_rel);
            removeWaiter(waiters, waiter);
            if (reserved != nullptr && reserved != WaiterNode::gaveUp()) {
                // Somebody handed a second resource over to us, give ours back
                returnResource(idle, nullptr);
                idle = reserved;
            }
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(idle, ResourceCloser(this)));
        }

        // Note the error code is not looked at: it only tells how we were woken up, the result is
        // decided by who wins the atomic exchange below
        co_await std::move(wait_result);

        ResourceType *reserved =
          waiter->handed.exchange(WaiterNode::gaveUp(), std::memory_order_acq_rel);
        removeWaiter(waiters, waiter);

        if (reserved != nullptr && reserved != WaiterNode::gaveUp()) {
            co_return stdx::expected<ResourcePtr, std::string>(
              ResourcePtr(reserved, ResourceCloser(this)));
        }

        co_return stdx::unexpected(timeout_msg);
    }

    /** The number of the currently active resources, i.e. all the resources (including the idle and
     *  the used ones) */
    size_t count() const {
        return m_count.load();
    }

    /**
     * The current number of the idle resources (the exact value)
     * It is tracked with an atomic counter to avoid operating the queue itself
     */
    size_t idleCount() const {
        return m_idleCount.load();
    }

    /** Release all the currently idle resources */
    void releaseIdleResource() {
        ResourceType *p = nullptr;
        while (m_resourceList.pop(p)) {
            if (p) {
                m_idleCount.fetch_sub(1);  // Decrease the idle count
                delete p;
                releaseResourceCount();  // Decrease the count and notify the destructor
            }
        }
    }

private:
    /** Pop one resource from the idle queue, nullptr when there is none; a resource whose version
     *  is too old is destroyed instead of being given out */
    ResourceType *takeIdleResource() {
        ResourceType *p = nullptr;
        if (m_resourceList.pop(p)) {
            m_idleCount.fetch_sub(1, std::memory_order_relaxed);
            if (p->getVersion() != m_version.load(std::memory_order_acquire)) {
                delete p;
                releaseResourceCount();
                return nullptr;
            }
            return p;
        }
        return nullptr;
    }

    /** Decrease the active resource count and notify the destructor waiting on it. The decrement
     *  may turn the destructor predicate true, so it is ordered against the waiting flag */
    void releaseResourceCount() {
        m_count.fetch_sub(1, std::memory_order_seq_cst);
        notifyCountChanged();
    }

    /** Notify the destructor that the resource counters have changed. The destructor is the only
     *  waiter on the condition variable, so while it is not waiting the notification is skipped to
     *  keep the resource returning path free of locks. The seq_cst order on both the flag and the
     *  counter operations closes the classic lost wakeup gap: either the notifier sees the flag
     *  (and then notifies after taking the mutex, which serializes with the waiting destructor) or
     *  the destructor sees the counter change when it evaluates its predicate */
    void notifyCountChanged() {
        if (m_destroy_waiting.load(std::memory_order_seq_cst)) {
            std::unique_lock<MutexType> lock(m_destroy_mutex);
            m_destroy_cv.notify_one();
        }
    }

    /** Join the waiting queue, false when it is full */
    bool pushWaiter(const std::shared_ptr<WaiterRegistry> &waiters,
                    const std::shared_ptr<WaiterNode> &waiter) {
        std::lock_guard<std::mutex> lock(waiters->mutex);
        if (waiters->queue.size() >= m_maxWaiters) {
            return false;
        }
        waiters->queue.emplace_back(waiter);
        m_registeredWaiters.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    /** Unregister our own waiter node, it is a no-op when somebody else already took it */
    void removeWaiter(const std::shared_ptr<WaiterRegistry> &waiters,
                      const std::shared_ptr<WaiterNode> &waiter) {
        std::lock_guard<std::mutex> lock(waiters->mutex);
        for (auto iter = waiters->queue.begin(); iter != waiters->queue.end(); ++iter) {
            if (iter->lock() == waiter) {
                waiters->queue.erase(iter);
                m_registeredWaiters.fetch_sub(1, std::memory_order_relaxed);
                return;
            }
        }
    }

    /** Hand a resource over to the first waiter that has not given up yet, true on success: the
     *  waiter owns the resource from then on, no matter what happens to the node afterwards.
     *  The node stays in the queue until its coroutine unregisters itself, so that the destructor
     *  can still take back a resource that nobody will claim */
    bool handToWaiter(ResourceType *p) {
        // The fast path of the resource returning: without any registered waiter there is nothing
        // to scan and no lock to take. A waiter registered just after this check finds the resource
        // by itself through the idle queue re-check below
        if (m_registeredWaiters.load(std::memory_order_acquire) == 0) {
            return false;
        }

        while (true) {
            std::shared_ptr<WaiterNode> waiter;
            {
                std::lock_guard<std::mutex> lock(m_waiters->mutex);
                for (auto iter = m_waiters->queue.begin(); iter != m_waiters->queue.end();) {
                    auto candidate = iter->lock();
                    if (!candidate) {
                        // The waiting coroutine is gone, clean its node up on the way
                        iter = m_waiters->queue.erase(iter);
                        m_registeredWaiters.fetch_sub(1, std::memory_order_relaxed);
                        continue;
                    }
                    ResourceType *current = candidate->handed.load(std::memory_order_relaxed);
                    if (current == WaiterNode::gaveUp()) {
                        // It has given up, clean its node up on the way
                        iter = m_waiters->queue.erase(iter);
                        m_registeredWaiters.fetch_sub(1, std::memory_order_relaxed);
                        continue;
                    }
                    if (current == nullptr) {
                        waiter = std::move(candidate);
                        break;
                    }
                    // A resource has been handed over to it but has not been taken yet
                    ++iter;
                }
                if (!waiter) {
                    return false;
                }
            }

            ResourceType *expected = nullptr;
            if (waiter->handed.compare_exchange_strong(expected, p, std::memory_order_acq_rel)) {
                // Wake it up with cancel(): it never touches the expiry, which the waiting
                // coroutine may rearm at the very same moment
                waiter->timer->cancel();
                return true;
            }

            // It has just given up, look for the next one
        }
    }

    /** Reserve a slot for a new resource with a CAS, false when the upper limit has been reached.
     *  Reserving first keeps the upper limit a hard one even when several threads create
     *  resources at the same time */
    bool reserveResourceSlot() {
        size_t current = m_count.load(std::memory_order_relaxed);
        while (m_maxCount == 0 || current < m_maxCount) {
            if (m_count.compare_exchange_weak(current, current + 1, std::memory_order_acq_rel)) {
                return true;
            }
        }
        return false;
    }

    class ResourceCloser {
    public:
        explicit ResourceCloser(ResourceAsioVersionPool *pool) : m_pool(pool) {}

        void operator()(ResourceType *conn) {
            if (conn) {
                // If the pool is bound, the resource is returned; otherwise it is deleted
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

    /** Return it to the resource pool */
    void returnResource(ResourceType *p, ResourceCloser *closer) {
        if (!p) [[unlikely]] {
            HKU_WARN("ResourceAsioVersionPool::returnResource: nullptr");
            return;
        }

        // If it is being destructed, delete the resource directly
        if (m_is_destroying.load(std::memory_order_acquire)) {
            delete p;
            releaseResourceCount();
            return;
        }

        // The returned resource is accepted only when its version equals the resource pool version
        if (p->getVersion() != m_version.load(std::memory_order_acquire)) {
            delete p;
            releaseResourceCount();
            return;
        }

        // Try to hand it over to a waiter that has not given up yet
        if (handToWaiter(p)) {
            return;
        }

        // There is no waiter to hand it over to, put it back into the idle queue
        if (!m_resourceList.push(p)) {
            // The queue is full, delete it directly
            delete p;
            releaseResourceCount();
            return;
        }

        m_idleCount.fetch_add(1, std::memory_order_seq_cst);
        notifyCountChanged();
    }

    std::atomic<size_t> m_count{0};      // The number of the currently active resources
    std::atomic<size_t> m_idleCount{0};  // The current number of the idle resources
    size_t m_maxCount;                   // The maximum resource upper limit
    Parameter m_param;
    boost::lockfree::queue<ResourceType *> m_resourceList;
    std::atomic<int> m_version{0};  // The current resource pool version

    mutable MutexType m_mutex;                  // The mutex protecting the parameter access
    size_t m_maxWaiters;                        // The maximum number of the waiters
    std::shared_ptr<WaiterRegistry> m_waiters;  // The waiting queue, shared with the waiting
                                                // coroutines
    MutexType m_destroy_mutex;                  // The mutex protecting the destructor waiting
                                                // condition variable
    std::condition_variable_any m_destroy_cv;   // Used to notify the destructor that a
                                                // resource has been returned
    std::atomic<bool> m_is_destroying{false};   // Marks whether it is being destructed
    std::atomic<size_t> m_registeredWaiters{
      0};  // The number of the registered waiters, it may be a little too large
    std::atomic<bool> m_destroy_waiting{
      false};  // Marks whether the destructor is waiting on the condition variable
};

}  // namespace hku

#endif /* HKU_UTILS_RESOURCE_ASIO_POOL_H */
