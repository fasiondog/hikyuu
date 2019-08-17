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

class HKU_API MySQLConnect: public DBConnectBase {
public:
    MySQLConnect(const Parameter& param);
    virtual ~MySQLConnect();

    virtual void exec(const string& sql_string);
    virtual SQLStatementPtr getStatement(const string& sql_statement);
    virtual bool tableExist(const string& tablename);

private:
    friend class MySQLStatement;
    shared_ptr<MYSQL> m_mysql;
};

} /* namespace */

#endif /* HIYUU_DB_CONNECT_MYSQL_MYSQLCONNECT_H */