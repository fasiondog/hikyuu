/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-01-19
 *     Author: fasiondog
 */

#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <memory>
#include "../utilities/Parameter.h"

namespace hku {

/**
 * Driver resource pool
 * @tparam DriverConnectT driver type, it is required to have the DriverType *clone() method
 * @ingroup DataDriver
 */
template <class DriverConnectT>
class DriverConnectPool {
public:
    DriverConnectPool() = delete;
    DriverConnectPool(const DriverConnectPool &) = delete;
    DriverConnectPool &operator=(const DriverConnectPool &) = delete;

    typedef typename DriverConnectT::DriverTypePtr DriverPtr;
    typedef std::shared_ptr<DriverConnectT> DriverConnectPtr;

    /**
     * Constructor
     * @param prototype driver prototype, the ownership is transferred to this pool
     * @param maxConnect the maximum number of connections allowed, 0 means unlimited
     * @param maxIdleConnect the maximum number of idle connections allowed, 0 means releasing
     *                       immediately, the default is the number of CPUs
     */
    explicit DriverConnectPool(const DriverPtr &prototype, size_t maxConnect = 0,
                               size_t maxIdleConnect = std::thread::hardware_concurrency())
    : m_maxSize(maxConnect),
      m_maxIdelSize(maxIdleConnect),
      m_count(0),
      m_prototype(prototype),
      m_closer(this) {}

    /**
     * Destructor, it releases all the cached connections
     */
    virtual ~DriverConnectPool() {
        while (!m_driverList.empty()) {
            DriverConnectT *p = m_driverList.front();
            m_driverList.pop();
            if (p) {
                delete p;
            }
        }
    }

    /** Get an available connection; if the maximum number of connections allowed is exceeded, it
     *  blocks and waits until an idle resource is obtained */
    DriverConnectPtr getConnect() {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_driverList.empty()) {
            if (m_maxSize > 0 && m_count >= m_maxSize) {
                m_cond.wait(lock, [this] { return !m_driverList.empty(); });
            } else {
                m_count++;
                return DriverConnectPtr(new DriverConnectT(m_prototype->clone()), m_closer);
            }
        }
        DriverConnectT *p = m_driverList.front();
        m_driverList.pop();
        return DriverConnectPtr(p, m_closer);
    }

    DriverPtr getPrototype() {
        return m_prototype;
    }

    /** Number of the currently active connections */
    size_t count() const {
        return m_count;
    }

    /** Number of the currently idle resources */
    size_t idleCount() const {
        return m_driverList.size();
    }

    /** Release all the currently idle resources */
    void releaseIdleConnect() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_driverList.empty()) {
            DriverConnectT *p = m_driverList.front();
            m_driverList.pop();
            m_count--;
            if (p) {
                delete p;
            }
        }
    }

private:
    /** Return it to the connection pool */
    void returnDriver(DriverConnectT *p) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (p) {
            if (m_driverList.size() < m_maxIdelSize) {
                m_driverList.push(p);
                m_cond.notify_all();
            } else {
                delete p;
                m_count--;
            }
        } else {
            m_count--;
            HKU_WARN("Trying to return an empty pointer!");
        }
    }

private:
    size_t m_maxSize;       // The maximum number of connections allowed
    size_t m_maxIdelSize;   // The maximum number of idle connections allowed
    size_t m_count;         // The number of currently active connections
    DriverPtr m_prototype;  // Driver prototype
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<DriverConnectT *> m_driverList;

    class DriverCloser {
    public:
        explicit DriverCloser(DriverConnectPool *pool) : m_pool(pool) {}
        void operator()(DriverConnectT *conn) {
            if (m_pool && conn) {
                m_pool->returnDriver(conn);
            }
        }

    private:
        DriverConnectPool *m_pool;
    };

    DriverCloser m_closer;
};

}  // namespace hku
