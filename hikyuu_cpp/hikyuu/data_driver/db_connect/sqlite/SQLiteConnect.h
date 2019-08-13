/*
 * SQLiteConnect.h
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-7-1
 *      Author: fasiondog
 */
#pragma once
#ifndef HIYUU_DB_CONNECT_SQLITE_SQLITECONNECT_H
#define HIYUU_DB_CONNECT_SQLITE_SQLITECONNECT_H

#include <sqlite3.h>
#include "../DBConnectPool.h"
#include "SQLiteStatement.h"

namespace hku {

/**
 * @defgroup SQLite SQLite3 driver SQLITE3 数据驱动
 * @ingroup DataDriver
 */

/**
 * @ingroup SQLite
 */
class HKU_API SQLiteConnect: public DBConnectBase {
public:
    SQLiteConnect(const Parameter& param);
    virtual ~SQLiteConnect() {}

    virtual void exec(const string& sql_string);
    virtual SQLStatementPtr getStatement(const string& sql_statement);
    virtual bool tableExist(const string& tablename);

private:
    friend class SQLiteStatement;

    string m_dbname;
    shared_ptr<sqlite3> m_db;
};

} /* namespace */

#endif /* HIYUU_DB_MANAGER_SQLITE_SQLITECONNECT_H */