/*
 * DBConnectPool.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-6
 *      Author: fasiondog
 */

#include "DBConnectPool.h"
#include "sqlite/SQLiteConnect.h"

namespace hku {

DBConnectPool::DBConnectPool(const Parameter& param) : m_param(param) {
    for (int i = 0; i < MIN_SIZE; i++) {
        m_connectList.push_back(make_shared<SQLiteConnect>(param));
    }
}

DBConnectPtr DBConnectPool::getConnect() noexcept {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_connectList.empty()) {
        return make_shared<SQLiteConnect>(m_param);
    }
    DBConnectPtr p = m_connectList.front();
    m_connectList.pop_front();
    return p;
}

void DBConnectPool::returnConnect(DBConnectPtr& p) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (p) {
        if (m_connectList.size() <= MAX_SIZE) {
            m_connectList.push_back(std::move(p));
        }
    } else {
        HKU_WARN("Trying to return an empty pointer!");
    }
}

}  // namespace hku