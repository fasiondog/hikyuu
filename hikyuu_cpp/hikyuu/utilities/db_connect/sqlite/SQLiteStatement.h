/*
 * SQLiteStatement.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-11
 *      Author: fasiondog
 */
#pragma once
#ifndef HIKYUU_DB_CONNECT_SQLITE_SQLITESTATEMENT_H
#define HIKYUU_DB_CONNECT_SQLITE_SQLITESTATEMENT_H

#include <sqlite3.h>
#include "../SQLStatementBase.h"

namespace hku {

/**
 * SQLite Statemen
 * @ingroup DBConnect
 */
class HKU_API SQLiteStatement : public SQLStatementBase {
public:
    SQLiteStatement() = delete;

    /**
     * 构造函数
     * @param driver 数据库连接
     * @param sql_statement SQL语句
     */
    SQLiteStatement(DBConnectBase *driver, const string &sql_statement);

    /** 析构函数 */
    virtual ~SQLiteStatement();

    virtual void sub_exec() override;
    virtual bool sub_moveNext() override;
    virtual uint64_t sub_getLastRowid() override;

    virtual void sub_bindNull(int idx) override;
    virtual void sub_bindInt(int idx, int64_t value) override;
    virtual void sub_bindDouble(int idx, double item) override;
    virtual void sub_bindDatetime(int idx, const Datetime &item) override;
    virtual void sub_bindText(int idx, const std::string &item) override;
    virtual void sub_bindText(int idx, const char *item, size_t len) override;
    virtual void sub_bindBlob(int idx, const std::string &item) override;
    virtual void sub_bindBlob(int idx, const std::vector<char> &item) override;

    virtual int sub_getNumColumns() const override;
    virtual void sub_getColumnAsInt64(int idx, int64_t &item) override;
    virtual void sub_getColumnAsDouble(int idx, double &item) override;
    virtual void sub_getColumnAsDatetime(int idx, Datetime &item) override;
    virtual void sub_getColumnAsText(int idx, std::string &item) override;
    virtual void sub_getColumnAsBlob(int idx, std::string &item) override;
    virtual void sub_getColumnAsBlob(int idx, std::vector<char> &item) override;

private:
    void _reset();

private:
    bool m_needs_reset;  // true if sqlite3_step() has been called more recently than
                         // sqlite3_reset()
    int m_step_status;
    bool m_at_first_step;
    sqlite3 *m_db;
    sqlite3_stmt *m_stmt;
};

}  // namespace hku

#endif /* HIKYUU_DB_CONNECT_SQLITE_SQLITESTATEMENT_H */