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
 * 驱动资源池
 * @tparam DriverType 驱动类型，要求具备 DriverType *clone() 方法
 * @ingroup DataDriver
 */
template <typename DriverType>
class DriverPool {
public:
    DriverPool() = delete;
    DriverPool(const DriverPool &) = delete;
    DriverPool &operator=(const DriverPool &) = delete;

    /**
     * 构造函数
     * @param param 驱动原型，所有权将被转移至该 pool
     * @param maxConnect 允许的最大连接数，为 0 表示不限制
     * @param maxIdleConnect 运行的最大空闲连接数，等于 0 时表示立刻释放
     */
    explicit DriverPool(const std::shared_ptr<DriverType> &prototype, size_t maxConnect = 0,
                        size_t maxIdleConnect = 100)
    : m_maxSize(maxConnect),
      m_maxIdelSize(maxIdleConnect),
      m_count(0),
      m_prototype(prototype),
      m_closer(this) {}

    /**
     * 析构函数，释放所有缓存的连接
     */
    virtual ~DriverPool() {
        while (!m_driverList.empty()) {
            DriverType *p = m_driverList.front();
            m_driverList.pop();
            if (p) {
                delete p;
            }
        }
    }

    /** 连接实例指针类型 */
    typedef std::shared_ptr<DriverType> DriverPtr;

    /** 获取可用连接，如超出允许的最大连接数将返回空指针 */
    DriverPtr get() noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_driverList.empty()) {
            if (m_maxSize > 0 && m_count >= m_maxSize) {
                HKU_WARN(
                  "There are no idle connections. The current maximum number of connections: {}",
                  m_maxSize);
                return DriverPtr();
            }
            m_count++;
            return DriverPtr(m_prototype->clone(), m_closer);
        }
        DriverType *p = m_driverList.front();
        m_driverList.pop();
        return DriverPtr(p, m_closer);
    }

    /**
     * 等待并获取连接，如果超出将抛出异常
     * @param timeout 超时时长（毫秒），如果等于或小于0，则表示不作超时限制
     * @return 连接指针
     */
    DriverPtr getAndWait(int timeout = 0) {
        int sleep = 100;
        int count = 0, max_count = timeout / sleep;
        DriverPtr result = get();
        while (!result) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
            result = get();
            if (timeout > 0) {
                YH_CHECK(count++ < max_count, "Can't get connect, timeout!");
            }
        }
        return result;
    }

    DriverPtr getPrototype() {
        return m_prototype;
    }

    /** 当前活动的连接数 */
    size_t count() const {
        return m_count;
    }

    /** 当前空闲的资源数 */
    size_t idleCount() const {
        return m_driverList.size();
    }

    /** 释放当前所有的空闲资源 */
    void releaseIdleConnect() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_driverList.empty()) {
            DriverType *p = m_driverList.front();
            m_driverList.pop();
            m_count--;
            if (p) {
                delete p;
            }
        }
    }

private:
    /** 归还至连接池 */
    void returnDriver(DriverType *p) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (p) {
            if (m_driverList.size() < m_maxIdelSize) {
                m_driverList.push(p);
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
    size_t m_maxSize;                         //允许的最大连接数
    size_t m_maxIdelSize;                     //允许的最大空闲连接数
    size_t m_count;                           //当前活动的连接数
    std::shared_ptr<DriverType> m_prototype;  // 驱动原型
    std::mutex m_mutex;
    std::queue<DriverType *> m_driverList;

    class DriverCloser {
    public:
        explicit DriverCloser(DriverPool *pool) : m_pool(pool) {}
        void operator()(DriverType *conn) {
            if (m_pool && conn) {
                m_pool->returnDriver(conn);
            }
        }

    private:
        DriverPool *m_pool;
    };

    DriverCloser m_closer;
};

}  // namespace hku
