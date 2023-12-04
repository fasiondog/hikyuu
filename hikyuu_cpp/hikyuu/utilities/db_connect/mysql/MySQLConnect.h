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

#if defined(_MSC_VER)
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

namespace hku {

class HKU_API MySQLConnect : public DBConnectBase {
public:
    explicit MySQLConnect(const Parameter& param);
    virtual ~MySQLConnect();

    MySQLConnect(const MySQLConnect&) = delete;
    MySQLConnect& operator=(const MySQLConnect&) = delete;

    virtual bool ping() override;

    virtual int64_t exec(const std::string& sql_string) override;
    virtual SQLStatementPtr getStatement(const std::string& sql_statement) override;
    virtual bool tableExist(const std::string& tablename) override;
    virtual void resetAutoIncrement(const std::string& tablename) override;

    virtual void transaction() noexcept override;
    virtual void commit() noexcept override;
    virtual void rollback() noexcept override;

private:
    bool tryConnect() noexcept;
    void connect();
    void close();

private:
    friend class MySQLStatement;
    MYSQL* m_mysql;
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_MYSQL_MYSQLCONNECT_H */