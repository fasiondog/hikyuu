/*
 * MySQLConnect.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#pragma once
#ifndef HIYUU_DB_CONNECT_MYSQL_MYSQLCONNECT_H
#define HIYUU_DB_CONNECT_MYSQL_MYSQLCONNECT_H

#include "../DBConnectBase.h"
#include "MySQLStatement.h"

#include <memory>

namespace hku {

class HKU_UTILS_API MySQLConnect : public DBConnectBase {
public:
    explicit MySQLConnect(const Parameter &param);
    virtual ~MySQLConnect() override;

    MySQLConnect(const MySQLConnect &) = delete;
    MySQLConnect &operator=(const MySQLConnect &) = delete;

    virtual bool ping() override;

    virtual int64_t exec(const std::string &sql_string) override;
    virtual SQLStatementPtr getStatement(const std::string &sql_statement) override;
    virtual bool tableExist(const std::string &tablename) override;
    virtual void resetAutoIncrement(const std::string &tablename) override;

    virtual void transaction() override;
    virtual void commit() override;
    virtual void rollback() noexcept override;

private:
    friend class MySQLStatement;

    // 提供给 MySQLStatement 访问原始连接的方法
    void *getRawConnection() const noexcept;

    // 内部辅助方法
    bool tryConnect() noexcept;
    void connect();
    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_MYSQL_MYSQLCONNECT_H */