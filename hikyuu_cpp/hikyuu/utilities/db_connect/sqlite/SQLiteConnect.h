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
#include "../DBConnectBase.h"
#include "SQLiteStatement.h"

namespace hku {

/**
 * @defgroup SQLite SQLite3 数据库连接
 * @ingroup DBConnect
 */

/**
 * @ingroup SQLite
 */
class HKU_API SQLiteConnect : public DBConnectBase {
public:
    SQLiteConnect(const Parameter& param);
    virtual ~SQLiteConnect() {}

    virtual void exec(const string& sql_string) override;
    virtual SQLStatementPtr getStatement(const string& sql_statement) override;
    virtual bool tableExist(const string& tablename) override;

private:
    friend class SQLiteStatement;

    string m_dbname;
    shared_ptr<sqlite3> m_db;
};

}  // namespace hku

#endif /* HIYUU_DB_MANAGER_SQLITE_SQLITECONNECT_H */