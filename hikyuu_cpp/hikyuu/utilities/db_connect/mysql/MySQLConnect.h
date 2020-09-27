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
    MySQLConnect(const Parameter& param) noexcept;
    virtual ~MySQLConnect();

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
    friend class MySQLStatement;
    MYSQL* m_mysql;
};

}  // namespace hku

#endif /* HIYUU_DB_CONNECT_MYSQL_MYSQLCONNECT_H */