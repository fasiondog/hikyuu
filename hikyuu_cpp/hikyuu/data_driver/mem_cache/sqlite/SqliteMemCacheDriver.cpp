/*
 *  Copyright(C) 2020 hikyuu.org
 *
 *  Create on: 2020-12-11
 *     Author: fasiondog
 */

#include "SqliteMemCacheDriver.h"

namespace hku {

SqliteMemCacheDriver::SqliteMemCacheDriver() : MemCacheDriver("sqlite") {}

bool SqliteMemCacheDriver::_init() {
    Parameter param;
    param.set<string>("db", "file:hikyuu_mem?mode=memory&cache=shared");
    m_pool = make_unique<ConnectPool<SQLiteConnect>>(param);
    return false;
}

}  // namespace hku