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
 * @defgroup SQLite SQLite3 driver SQLITE3 数据驱动
 * @ingroup DBConnect
 */

/**
 * SQLite连接
 * @ingroup SQLite
 */
class HKU_API SQLiteConnect : public DBConnectBase {
public:
    /**
     * 构造函数
     * @param param 数据库连接参数,支持如下参数：
     * <pre>
     * string db - 数据库文件名
     * int flags - SQLite连接方式：SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX
     *            （具体可参考 SQLite 帮助）
     * </pre>
     */
    explicit SQLiteConnect(const Parameter& param);

    /** 析构函数 */
    virtual ~SQLiteConnect();

    virtual bool ping() override;
    virtual void exec(const string& sql_string) override;
    virtual SQLStatementPtr getStatement(const string& sql_statement) override;
    virtual bool tableExist(const string& tablename) override;

    virtual void transaction() override;
    virtual void commit() override;
    virtual void rollback() override;

private:
    void close();

private:
    friend class SQLiteStatement;
    string m_dbname;
    sqlite3* m_db;
};

typedef shared_ptr<SQLiteConnect> SQLiteConnectPtr;

}  // namespace hku

#endif /* HIYUU_DB_MANAGER_SQLITE_SQLITECONNECT_H */