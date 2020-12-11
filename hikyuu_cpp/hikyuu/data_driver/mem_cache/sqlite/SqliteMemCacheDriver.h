/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-11
 *     Author: fasiondog
 */

#pragma once

#include "../../../utilities/db_connect/sqlite/SQLiteConnect.h"
#include "../../../utilities/ConnectPool.h"
#include "../../MemCacheDriver.h"

namespace hku {

class SqliteMemCacheDriver : public MemCacheDriver {
public:
    SqliteMemCacheDriver();
    virtual ~SqliteMemCacheDriver() = default;

    virtual bool _init() override;

private:
    unique_ptr<ConnectPool<SQLiteConnect>> m_pool;
};

}  // namespace hku