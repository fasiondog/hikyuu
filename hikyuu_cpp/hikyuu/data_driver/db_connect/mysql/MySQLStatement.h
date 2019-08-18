/*
 * MySQLStatement.h
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#pragma once
#ifndef HIYUU_DB_CONNECT_MYSQL_MYSQLSTATEMENT_H
#define HIYUU_DB_CONNECT_MYSQL_MYSQLSTATEMENT_H

#include <boost/any.hpp>
#include "../SQLStatementBase.h"

#if defined(_MSC_VER)
    #include <mysql.h>
#else
    #include <mysql/mysql.h>
#endif

namespace hku {

class HKU_API MySQLStatement: public SQLStatementBase {
public:
    MySQLStatement() = delete;
    MySQLStatement(const DBConnectPtr& driver, const string& sql_statement);
    virtual ~MySQLStatement();

    virtual bool sub_isValid() const;
    virtual void sub_exec();
    virtual bool sub_moveNext();

    virtual void sub_bindNull(int idx);
    virtual void sub_bindInt(int idx, int64 value);
    virtual void sub_bindDouble(int idx, double item);
    virtual void sub_bindText(int idx, const string& item);
    virtual void sub_bindBlob(int idx, const string& item);

    virtual int sub_getNumColumns() const;
    virtual void sub_getColumnAsInt64(int idx, int64& item);
    virtual void sub_getColumnAsDouble(int idx, double& item);
    virtual void sub_getColumnAsText(int idx, string& item);
    virtual void sub_getColumnAsBlob(int idx, string& item);

private:
    void _reset();

private:
    shared_ptr<MYSQL> m_db;
    MYSQL_STMT *m_stmt;
    bool m_needs_reset;
    vector<MYSQL_BIND> m_param_bind;
    vector<MYSQL_BIND> m_result_bind;
    vector<boost::any> m_param_buffer;
};

} /* namespace */

#endif /* HIYUU_DB_CONNECT_MYSQL_MYSQLSTATEMENT_H */