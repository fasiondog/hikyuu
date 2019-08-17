/*
 * MySQLStatement.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 * 
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#include "MySQLStatement.h"
#include "MySQLConnect.h"

namespace hku {

MySQLStatement::MySQLStatement(const DBConnectPtr& driver, const string& sql_statement)
: SQLStatementBase(driver, sql_statement), 
  m_stmt(nullptr), 
  m_needs_reset(false) {
    m_db = (dynamic_cast<MySQLConnect *>(driver.get()))->m_mysql;
    m_stmt = mysql_stmt_init(m_db.get());
    HKU_ASSERT_M(m_stmt != nullptr, "Failed mysql_stmt_init!");
    int ret = mysql_stmt_prepare(m_stmt, sql_statement.c_str(), sql_statement.size());
    if (ret != 0) {
        mysql_stmt_close(m_stmt);
        m_stmt = nullptr;
        HKU_THROW("Failed prepare statement! error: {} SQL: {}", 
                  mysql_stmt_error(m_stmt), sql_statement);
    }
}

MySQLStatement::~MySQLStatement() {
    mysql_stmt_close(m_stmt);
}

bool MySQLStatement::sub_isValid() const {
    return m_stmt ? true : false;
}

void MySQLStatement::_reset() {
    if (m_needs_reset) {
        int ret = mysql_stmt_reset(m_stmt);
        HKU_ASSERT_M(ret == 0, "Failed reset statement! {}", mysql_stmt_error(m_stmt));
    }
}

void MySQLStatement::sub_exec() {
    _reset();
}

bool MySQLStatement::sub_moveNext() {
    return false;
}

void MySQLStatement::sub_bindNull(int idx) {

}

void MySQLStatement::sub_bindInt(int idx, int64 value) {
    MYSQL_BIND bind;
    m_bind.push_back(bind);
}

void MySQLStatement::sub_bindDouble(int idx, double item) {

}

void MySQLStatement::sub_bindText(int idx, const string& item) {
    
}

void MySQLStatement::sub_bindBlob(int idx, const string& item) {

}

int MySQLStatement::sub_getNumColumns() const {
    return 0;
}

void MySQLStatement::sub_getColumnAsInt64(int idx, int64& item) {

}

void MySQLStatement::sub_getColumnAsDouble(int idx, double& item) {

}

void MySQLStatement::sub_getColumnAsText(int idx, string& item) {

}

void MySQLStatement::sub_getColumnAsBlob(int idx, string& item) {
    
}

} /* namespace */