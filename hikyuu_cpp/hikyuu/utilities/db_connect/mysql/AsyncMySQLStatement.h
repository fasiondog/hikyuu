/*
 * AsyncMySQLStatement.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */

#pragma once
#ifndef HIYUU_DB_CONNECT_MYSQL_ASYNCMYSQLSTATEMENT_H
#define HIYUU_DB_CONNECT_MYSQL_ASYNCMYSQLSTATEMENT_H

#include "../AsyncSQLStatementBase.h"

#include <memory>

namespace hku {

// 前向声明，避免循环依赖
class AsyncMySQLConnect;

class HKU_UTILS_API AsyncMySQLStatement : public AsyncSQLStatementBase {
public:
    AsyncMySQLStatement(AsyncMySQLConnect* connect, const std::string &sql);
    virtual ~AsyncMySQLStatement() override;

    AsyncMySQLStatement(const AsyncMySQLStatement &) = delete;
    AsyncMySQLStatement &operator=(const AsyncMySQLStatement &) = delete;

    virtual net::awaitable<void> sub_exec() override;
    virtual net::awaitable<bool> sub_moveNext() override;
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
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_MYSQL_ASYNCMYSQLSTATEMENT_H */
