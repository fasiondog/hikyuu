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
  m_meta_result(nullptr),
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

    auto param_count = mysql_stmt_param_count(m_stmt);
    if (param_count > 0) {
        m_param_bind.resize(param_count);
        memset(m_param_bind.data(), 0, param_count * sizeof(MYSQL_BIND));
    }

    MYSQL_RES *m_meta_result = mysql_stmt_result_metadata(m_stmt);
    if (m_meta_result) {
        int column_count = mysql_num_fields(m_meta_result);
        m_result_bind.resize(column_count);
        memset(m_result_bind.data(), 0, column_count * sizeof(MYSQL_BIND));
    }
}

MySQLStatement::~MySQLStatement() {
    if (m_meta_result) {
        mysql_free_result(m_meta_result);
    }
    mysql_stmt_close(m_stmt);
}

bool MySQLStatement::sub_isValid() const {
    return m_stmt ? true : false;
}

void MySQLStatement::_reset() {
    if (m_needs_reset) {
        int ret = mysql_stmt_reset(m_stmt);
        HKU_ASSERT_M(ret == 0, "Failed reset statement! {}", mysql_stmt_error(m_stmt));
        m_param_bind.clear();
        m_result_bind.clear();
        m_needs_reset = false;
    }
}

void MySQLStatement::sub_exec() {
    _reset();
    HKU_ASSERT_M(
        mysql_stmt_execute(m_stmt)==0, 
        "Failed mysql_stmt_execute: {}", 
        mysql_stmt_error(m_stmt)
    );
    m_needs_reset = true;
    
}

bool MySQLStatement::sub_moveNext() {
    return false;
}

void MySQLStatement::sub_bindNull(int idx) {
    HKU_ASSERT_M(
        idx < m_param_bind.size(), 
        "idx out of range! idx: {}, total: {}", 
        idx, m_param_bind.size()
    );
    m_param_bind[idx].buffer_type= MYSQL_TYPE_NULL;
}

void MySQLStatement::sub_bindInt(int idx, int64 value) {
    HKU_ASSERT_M(
        idx < m_param_bind.size(), 
        "idx out of range! idx: {}, total: {}", 
        idx, m_param_bind.size()
    );
    m_param_buffer.push_back(value);
    auto& buf = m_param_buffer.back();
    m_param_bind[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    m_param_bind[idx].buffer = boost::any_cast<int64>(&buf);
}

void MySQLStatement::sub_bindDouble(int idx, double item) {
    HKU_ASSERT_M(
        idx < m_param_bind.size(), 
        "idx out of range! idx: {}, total: {}", 
        idx, m_param_bind.size()
    );
    m_param_buffer.push_back(item);
    auto& buf = m_param_buffer.back();
    m_param_bind[idx].buffer_type = MYSQL_TYPE_DOUBLE;
    m_param_bind[idx].buffer = boost::any_cast<double>(&buf);
}

void MySQLStatement::sub_bindText(int idx, const string& item) {
    HKU_ASSERT_M(
        idx < m_param_bind.size(), 
        "idx out of range! idx: {}, total: {}", 
        idx, m_param_bind.size()
    );
    m_param_buffer.push_back(item);
    auto& buf = m_param_buffer.back();
    string *p = boost::any_cast<string>(&buf);
    m_param_bind[idx].buffer_type = MYSQL_TYPE_STRING;
    m_param_bind[idx].buffer = (void *)p->data();
    m_param_bind[idx].buffer_length = item.size();
    m_param_bind[idx].is_null = 0;
    
    unsigned long str_len = item.size();
    m_param_buffer.push_back(str_len);
    auto& ref = m_param_buffer.back();
    m_param_bind[idx].length = boost::any_cast<unsigned long>(&ref);;
}

void MySQLStatement::sub_bindBlob(int idx, const string& item) {
    HKU_ASSERT_M(
        idx < m_param_bind.size(), 
        "idx out of range! idx: {}, total: {}", 
        idx, m_param_bind.size()
    );
    m_param_buffer.push_back(item);
    auto& buf = m_param_buffer.back();
    string *p = boost::any_cast<string>(&buf);
    m_param_bind[idx].buffer_type = MYSQL_TYPE_BLOB;
    m_param_bind[idx].buffer = (void *)p->data();
    m_param_bind[idx].buffer_length = item.size();
    m_param_bind[idx].is_null = 0;
    
    unsigned long str_len = item.size();
    m_param_buffer.push_back(str_len);
    auto& ref = m_param_buffer.back();
    m_param_bind[idx].length = boost::any_cast<unsigned long>(&ref);;
}

int MySQLStatement::sub_getNumColumns() const {
    return mysql_stmt_field_count(m_stmt);
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