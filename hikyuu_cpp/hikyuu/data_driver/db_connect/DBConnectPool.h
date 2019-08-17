/*
 * DBConnectPool.h
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-8-5
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_DBCONNECTPOOL_H
#define HIKYUU_DB_CONNECT_DBCONNECTPOOL_H

#include <thread>
#include "DBConnectBase.h"

namespace hku {

/**
 * 数据驱动池（连接池）
 * @ingroup DataDriver
 */
class HKU_API DBConnectPool {
public:
    /**
     * 构造函数
     * @param param 驱动参数
     * @param size 驱动池大小
     */
    DBConnectPool(const Parameter& param, int size=10);

    virtual ~DBConnectPool() = default;

    /** 获取数据驱动，如果当前无可用驱动，将返回空指针 */
    DBConnectPtr getConnect() noexcept;

    /** 归还数据驱动至驱动池 */
    void returnConnect(DBConnectPtr& p);

private:
    Parameter m_param;
    std::mutex m_mutex;
    std::list<DBConnectPtr> m_connectList;

private:
    DBConnectPool() = delete;
    DBConnectPool(const DBConnectPool&) = delete;
    DBConnectPool& operator=(const DBConnectPool&) = delete;
};


/**
 * 从池中获取驱动辅助工具，以保证退出作用域时能够及时归还驱动至驱动池
 * @ingroup DataDriver
 */
struct DBConnectGuard {
    DBConnectGuard(DBConnectPool *pool): m_pool(pool) {
        if (m_pool) {
            m_driver = m_pool->getConnect();
        }
    }

    ~DBConnectGuard() {
        if (m_pool && m_driver) {
            m_pool->returnConnect(m_driver);
        }
    }

    DBConnectPtr getConnect() {
        return m_driver;
    }

private:
    DBConnectPtr m_driver;
    DBConnectPool *m_pool;
};

} /* namespace */

#endif /* HIKYUU_DB_CONNECT_DBCONNECTPOOL_H */