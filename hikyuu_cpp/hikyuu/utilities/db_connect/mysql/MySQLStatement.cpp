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
  m_db((dynamic_cast<MySQLConnect*>(driver.get()))->m_mysql),
  m_stmt(nullptr),
  m_meta_result(nullptr),
  m_needs_reset(false),
  m_has_bind_result(false) {
    m_stmt = mysql_stmt_init(m_db.get());
    HKU_ASSERT_M(m_stmt != nullptr, "Failed mysql_stmt_init!");
    int ret = mysql_stmt_prepare(m_stmt, sql_statement.c_str(), sql_statement.size());
    if (ret != 0) {
        std::string stmt_errorstr = mysql_stmt_error(m_stmt);
        mysql_stmt_close(m_stmt);
        m_stmt = nullptr;
        HKU_THROW("Failed prepare statement! error: {} SQL: {}", stmt_errorstr.c_str(),
                  sql_statement);
    }

    auto param_count = mysql_stmt_param_count(m_stmt);
    if (param_count > 0) {
        m_param_bind.resize(param_count);
        memset(m_param_bind.data(), 0, param_count * sizeof(MYSQL_BIND));
    }

    m_meta_result = mysql_stmt_result_metadata(m_stmt);
    if (m_meta_result) {
        int column_count = mysql_num_fields(m_meta_result);
        m_result_bind.resize(column_count);
        memset(m_result_bind.data(), 0, column_count * sizeof(MYSQL_BIND));
        m_result_length.resize(column_count, 0);
        m_result_is_null.resize(column_count, 0);
        m_result_error.resize(column_count, 0);
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
        // m_param_bind.clear();
        // m_result_bind.clear();
        m_param_buffer.clear();
        m_result_buffer.clear();
        m_needs_reset = false;
        m_has_bind_result = false;
    }
}

void MySQLStatement::sub_exec() {
    _reset();
    m_needs_reset = true;
    if (m_param_bind.size() > 0) {
        HKU_ASSERT_M(mysql_stmt_bind_param(m_stmt, m_param_bind.data()) == 0,
                     "Failed mysql_stmt_bind_param! {}", mysql_stmt_error(m_stmt));
    }
    HKU_ASSERT_M(mysql_stmt_execute(m_stmt) == 0, "Failed mysql_stmt_execute: {}",
                 mysql_stmt_error(m_stmt));
}

void MySQLStatement::_bindResult() {
    if (!m_meta_result) {
        return;
    }

    MYSQL_FIELD* field;
    int idx = 0;
    while ((field = mysql_fetch_field(m_meta_result))) {
        m_result_bind[idx].buffer_type = field->type;
#if MYSQL_VERSION_ID >= 80000
        m_result_bind[idx].is_null = (bool*)&m_result_is_null[idx];
        m_result_bind[idx].error = (bool*)&m_result_error[idx];
#else
        m_result_bind[idx].is_null = &m_result_is_null[idx];
        m_result_bind[idx].error = &m_result_error[idx];
#endif
        m_result_bind[idx].length = &m_result_length[idx];

        if (field->type == MYSQL_TYPE_LONGLONG) {
            int64 item = 0;
            m_result_buffer.push_back(item);
            auto& buf = m_result_buffer.back();
            m_result_bind[idx].buffer = boost::any_cast<int64>(&buf);
        } else if (field->type == MYSQL_TYPE_DOUBLE) {
            double item = 0;
            m_result_buffer.push_back(item);
            auto& buf = m_result_buffer.back();
            m_result_bind[idx].buffer = boost::any_cast<double>(&buf);
        } else if (field->type == MYSQL_TYPE_VAR_STRING || field->type == MYSQL_TYPE_BLOB) {
            m_result_bind[idx].buffer_length = 4096;
            vector<char> item(4096);
            m_result_buffer.push_back(item);
            auto& buf = m_result_buffer.back();
            vector<char>* p = boost::any_cast<vector<char>>(&buf);
            m_result_bind[idx].buffer = p->data();
        }

        idx++;
    }
}

bool MySQLStatement::sub_moveNext() {
    if (!m_has_bind_result) {
        _bindResult();
        m_has_bind_result = true;

        HKU_ASSERT_M(mysql_stmt_bind_result(m_stmt, m_result_bind.data()) == 0,
                     "Failed mysql_stmt_bind_result! {}", mysql_stmt_error(m_stmt));

        HKU_ASSERT_M(mysql_stmt_store_result(m_stmt) == 0, "Failed mysql_stmt_store_result! {}",
                     mysql_stmt_error(m_stmt));
    }

    int ret = mysql_stmt_fetch(m_stmt);
    if (ret == 0) {
        return true;
    } else if (ret == 1) {
        HKU_THROW("Error occurred in mysql_stmt_fetch! {}", mysql_stmt_error(m_stmt));
    }
    return false;
}

void MySQLStatement::sub_bindNull(int idx) {
    HKU_ASSERT_M(idx < m_param_bind.size(), "idx out of range! idx: {}, total: {}", idx,
                 m_param_bind.size());
    m_param_bind[idx].buffer_type = MYSQL_TYPE_NULL;
}

void MySQLStatement::sub_bindInt(int idx, int64 value) {
    HKU_ASSERT_M(idx < m_param_bind.size(), "idx out of range! idx: {}, total: {}", idx,
                 m_param_bind.size());
    m_param_buffer.push_back(value);
    auto& buf = m_param_buffer.back();
    m_param_bind[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    m_param_bind[idx].buffer = boost::any_cast<int64>(&buf);
}

void MySQLStatement::sub_bindDouble(int idx, double item) {
    HKU_ASSERT_M(idx < m_param_bind.size(), "idx out of range! idx: {}, total: {}", idx,
                 m_param_bind.size());
    m_param_buffer.push_back(item);
    auto& buf = m_param_buffer.back();
    m_param_bind[idx].buffer_type = MYSQL_TYPE_DOUBLE;
    m_param_bind[idx].buffer = boost::any_cast<double>(&buf);
}

void MySQLStatement::sub_bindText(int idx, const string& item) {
    HKU_ASSERT_M(idx < m_param_bind.size(), "idx out of range! idx: {}, total: {}", idx,
                 m_param_bind.size());
    m_param_buffer.push_back(item);
    auto& buf = m_param_buffer.back();
    string* p = boost::any_cast<string>(&buf);
    m_param_bind[idx].buffer_type = MYSQL_TYPE_VAR_STRING;
    m_param_bind[idx].buffer = (void*)p->data();
    m_param_bind[idx].buffer_length = item.size();
    m_param_bind[idx].is_null = 0;

    unsigned long str_len = item.size();
    m_param_buffer.push_back(str_len);
    auto& ref = m_param_buffer.back();
    m_param_bind[idx].length = boost::any_cast<unsigned long>(&ref);
    ;
}

void MySQLStatement::sub_bindBlob(int idx, const string& item) {
    HKU_ASSERT_M(idx < m_param_bind.size(), "idx out of range! idx: {}, total: {}", idx,
                 m_param_bind.size());
    m_param_buffer.push_back(item);
    auto& buf = m_param_buffer.back();
    string* p = boost::any_cast<string>(&buf);
    m_param_bind[idx].buffer_type = MYSQL_TYPE_BLOB;
    m_param_bind[idx].buffer = (void*)p->data();
    m_param_bind[idx].buffer_length = item.size();
    m_param_bind[idx].is_null = 0;

    unsigned long str_len = item.size();
    m_param_buffer.push_back(str_len);
    auto& ref = m_param_buffer.back();
    m_param_bind[idx].length = boost::any_cast<unsigned long>(&ref);
    ;
}

int MySQLStatement::sub_getNumColumns() const {
    return mysql_stmt_field_count(m_stmt);
}

void MySQLStatement::sub_getColumnAsInt64(int idx, int64& item) {
    HKU_ASSERT_M(idx < m_result_buffer.size(), "idx out of range! idx: {}, total: {}",
                 m_result_buffer.size());

    HKU_ASSERT_M(m_result_error[idx] == 0, "Error occurred in sub_getColumnAsInt64! idx: {}", idx);

    if (m_result_is_null[idx]) {
        item = 0;
        return;
    }

    try {
        item = boost::any_cast<int64>(m_result_buffer[idx]);
    } catch (...) {
        HKU_THROW("Field type mismatch! idx: {}", idx);
    }
}

void MySQLStatement::sub_getColumnAsDouble(int idx, double& item) {
    HKU_ASSERT_M(idx < m_result_buffer.size(), "idx out of range! idx: {}, total: {}",
                 m_result_buffer.size());

    HKU_ASSERT_M(m_result_error[idx] == 0, "Error occurred in sub_getColumnAsDouble! idx: {}", idx);

    if (m_result_is_null[idx]) {
        item = 0;
        return;
    }

    try {
        item = boost::any_cast<double>(m_result_buffer[idx]);
    } catch (...) {
        HKU_THROW("Field type mismatch! idx: {}", idx);
    }
}

void MySQLStatement::sub_getColumnAsText(int idx, string& item) {
    HKU_ASSERT_M(idx < m_result_buffer.size(), "idx out of range! idx: {}, total: {}",
                 m_result_buffer.size());

    HKU_ASSERT_M(m_result_error[idx] == 0, "Error occurred in sub_getColumnAsText! idx: {}", idx);

    if (m_result_is_null[idx]) {
        item.clear();
        return;
    }

    try {
        vector<char>* p = boost::any_cast<vector<char>*>(m_result_buffer[idx]);
        std::ostringstream buf;
        for (unsigned long i = 0; i < m_result_length[idx]; i++) {
            buf << (*p)[i];
        }
        item = buf.str();
    } catch (...) {
        HKU_THROW("Field type mismatch! idx: {}", idx);
    }
}

void MySQLStatement::sub_getColumnAsBlob(int idx, string& item) {
    HKU_ASSERT_M(idx < m_result_buffer.size(), "idx out of range! idx: {}, total: {}",
                 m_result_buffer.size());

    HKU_ASSERT_M(m_result_error[idx] == 0, "Error occurred in sub_getColumnAsBlob! idx: {}", idx);

    if (m_result_is_null[idx]) {
        item.clear();
        return;
    }

    try {
        vector<char>* p = boost::any_cast<vector<char>*>(m_result_buffer[idx]);
        std::ostringstream buf;
        for (unsigned long i = 0; i < m_result_length[idx]; i++) {
            buf << (*p)[i];
        }
        item = buf.str();
    } catch (...) {
        HKU_THROW("Field type mismatch! idx: {}", idx);
    }
}

}  // namespace hku
