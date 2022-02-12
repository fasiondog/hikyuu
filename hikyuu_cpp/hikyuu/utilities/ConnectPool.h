/*
 * ConnectPool.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-5
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_UTILITIES_CONNECTPOOL_H
#define HIKYUU_UTILITIES_CONNECTPOOL_H

#include <thread>
#include <mutex>
#include <queue>
#include "Parameter.h"

namespace hku {

/**
 * 通用连接池
 * @ingroup Utilities
 */
template <typename ConnectType>
class ConnectPool {
public:
    ConnectPool() = delete;
    ConnectPool(const ConnectPool &) = delete;
    ConnectPool &operator=(const ConnectPool &) = delete;

    /**
     * 构造函数
     * @param param 连接参数
     * @param maxConnect 允许的最大连接数，为 0 表示不限制
     * @param maxIdleConnect 运行的最大空闲连接数，等于 0 时表示立刻释放
     */
    explicit ConnectPool(const Parameter &param, size_t maxConnect = 0, size_t maxIdleConnect = 100)
    : m_maxConnectSize(maxConnect),
      m_maxIdelSize(maxIdleConnect),
      m_count(0),
      m_param(param),
      m_closer(this) {}

    /**
     * 析构函数，释放所有缓存的连接
     */
    virtual ~ConnectPool() {
        while (!m_connectList.empty()) {
            ConnectType *p = m_connectList.front();
            m_connectList.pop();
            if (p) {
                delete p;
            }
        }
    }

    /** 连接实例指针类型 */
    typedef std::shared_ptr<ConnectType> ConnectPtr;

    /** 获取可用连接，如超出允许的最大连接数将返回空指针 */
    ConnectPtr getConnect() noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_connectList.empty()) {
            if (m_maxConnectSize > 0 && m_count >= m_maxConnectSize) {
                HKU_WARN(
                  "There are no idle connections. The current maximum number of connections: {}",
                  m_maxConnectSize);
                return ConnectPtr();
            }
            m_count++;
            return ConnectPtr(new ConnectType(m_param), m_closer);
        }
        ConnectType *p = m_connectList.front();
        m_connectList.pop();
        return ConnectPtr(p, m_closer);
    }

    /**
     * 等待并获取连接，如果超出将抛出异常
     * @param timeout 超时时长（毫秒），如果等于或小于0，则表示不作超时限制
     * @return 连接指针
     */
    ConnectPtr getAndWait(int timeout = 0) {
        int sleep = 100;
        int count = 0, max_count = timeout / sleep;
        ConnectPtr result = getConnect();
        while (!result) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
            result = getConnect();
            if (timeout > 0) {
                HKU_CHECK(count++ < max_count, "Can't get connect, timeout!");
            }
        }
        return result;
    }

    /** 当前活动的连接数 */
    size_t count() const {
        return m_count;
    }

    /** 当前空闲的资源数 */
    size_t idleCount() const {
        return m_connectList.size();
    }

    /** 释放当前所有的空闲资源 */
    void releaseIdleConnect() {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_connectList.empty()) {
            ConnectType *p = m_connectList.front();
            m_connectList.pop();
            m_count--;
            if (p) {
                delete p;
            }
        }
    }

private:
    /** 归还至连接池 */
    void returnDriver(ConnectType *p) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (p) {
            if (m_connectList.size() < m_maxIdelSize) {
                m_connectList.push(p);
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
    size_t m_maxConnectSize;  //允许的最大连接数
    size_t m_maxIdelSize;     //允许的最大空闲连接数
    size_t m_count;           //当前活动的连接数
    Parameter m_param;
    std::mutex m_mutex;
    std::queue<ConnectType *> m_connectList;

    class ConnectCloser {
    public:
        explicit ConnectCloser(ConnectPool *pool) : m_pool(pool) {}
        void operator()(ConnectType *conn) {
            if (m_pool && conn) {
                m_pool->returnDriver(conn);
            }
        }

    private:
        ConnectPool *m_pool;
    };

    ConnectCloser m_closer;
};

}  // namespace hku

#endif /* HIKYUU_UTILITIES_CONNECTPOOL_H */