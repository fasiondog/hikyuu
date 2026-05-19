/*
 * AsyncMySQLConnect.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */

#pragma once
#ifndef HIYUU_DB_CONNECT_MYSQL_ASYNCMYSQLCONNECT_H
#define HIYUU_DB_CONNECT_MYSQL_ASYNCMYSQLCONNECT_H

#include "hikyuu/utilities/Parameter.h"
#include "../AsyncDBConnectBase.h"
#include "AsyncMySQLStatement.h"

#include <memory>

namespace hku {

class HKU_UTILS_API AsyncMySQLConnect : public AsyncDBConnectBase {
public:
    explicit AsyncMySQLConnect(const Parameter &param);
    virtual ~AsyncMySQLConnect() override;

    AsyncMySQLConnect(const AsyncMySQLConnect &) = delete;
    AsyncMySQLConnect &operator=(const AsyncMySQLConnect &) = delete;

    virtual net::awaitable<bool> ping() override;
    virtual net::awaitable<int64_t> exec(const std::string &sql_string) override;
    virtual net::awaitable<AsyncSQLStatementPtr> getStatement(
      const std::string &sql_statement) override;
    virtual net::awaitable<bool> tableExist(const std::string &tablename) override;
    virtual net::awaitable<void> resetAutoIncrement(const std::string &tablename) override;

    virtual net::awaitable<void> transaction() override;
    virtual net::awaitable<void> commit() override;
    virtual net::awaitable<void> rollback() noexcept override;

private:
    friend class AsyncMySQLStatement;

    // 提供给 AsyncMySQLStatement 访问原始连接的方法（使用 void* 避免暴露 boost.mysql 类型）
    void *getRawConnection() const;

    // 内部辅助方法
    net::awaitable<bool> tryConnect();
    net::awaitable<void> connect();
    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_MYSQL_ASYNCMYSQLCONNECT_H */
