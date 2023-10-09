/*
 * SQLiteStatement.cpp
 *
 *  Copyright (c) 2019, hiyuu.org
 *
 *  Created on: 2019-7-11
 *      Author: fasiondog
 */

#include "SQLiteStatement.h"
#include "SQLiteConnect.h"

namespace hku {

SQLiteStatement::SQLiteStatement(DBConnectBase* driver, const string& sql_statement)
: SQLStatementBase(driver, sql_statement),
  m_needs_reset(false),
  m_step_status(SQLITE_DONE),
  m_at_first_step(true),
  m_db((dynamic_cast<SQLiteConnect*>(driver))->m_db),
  m_stmt(NULL) {
    int status =
      sqlite3_prepare_v2(m_db, m_sql_string.c_str(), m_sql_string.size() + 1, &m_stmt, NULL);
    if (status != SQLITE_OK) {
        sqlite3_finalize(m_stmt);
        SQL_THROW(status, "Failed prepare sql statement: {}! error msg: {}", m_sql_string,
                  sqlite3_errmsg(m_db));
    }

    HKU_CHECK(m_stmt != 0, "Invalid SQL statement: {}", m_sql_string);
}

SQLiteStatement::~SQLiteStatement() {
    sqlite3_finalize(m_stmt);
}

void SQLiteStatement::_reset() {
    if (m_needs_reset) {
        int status = sqlite3_reset(m_stmt);
        if (status != SQLITE_OK) {
            m_step_status = SQLITE_DONE;
            SQL_THROW(status, sqlite3_errmsg(m_db));
        }
        m_needs_reset = false;
        m_step_status = SQLITE_DONE;
        m_at_first_step = true;
    }
}

void SQLiteStatement::sub_exec() {
    _reset();
    m_step_status = sqlite3_step(m_stmt);
    m_needs_reset = true;
    if (m_step_status != SQLITE_DONE && m_step_status != SQLITE_ROW) {
        SQL_THROW(m_step_status, sqlite3_errmsg(m_db));
    }
}

bool SQLiteStatement::sub_moveNext() {
    if (m_step_status == SQLITE_ROW) {
        if (m_at_first_step) {
            m_at_first_step = false;
            return true;
        } else {
            m_step_status = sqlite3_step(m_stmt);
            if (m_step_status == SQLITE_DONE) {
                return false;
            } else if (m_step_status == SQLITE_ROW) {
                return true;
            } else {
                SQL_THROW(m_step_status, sqlite3_errmsg(m_db));
            }
        }
    } else {
        return false;
    }
}

int SQLiteStatement::sub_getNumColumns() const {
    return (m_at_first_step == false) && (m_step_status == SQLITE_ROW)
             ? sqlite3_column_count(m_stmt)
             : 0;
}

void SQLiteStatement::sub_bindNull(int idx) {
    _reset();
    int status = sqlite3_bind_null(m_stmt, idx + 1);
    SQL_CHECK(status == SQLITE_OK, status, sqlite3_errmsg(m_db));
}

void SQLiteStatement::sub_bindInt(int idx, int64_t value) {
    _reset();
    int status = sqlite3_bind_int64(m_stmt, idx + 1, value);
    SQL_CHECK(status == SQLITE_OK, status, sqlite3_errmsg(m_db));
}

void SQLiteStatement::sub_bindText(int idx, const string& item) {
    _reset();
    int status = sqlite3_bind_text(m_stmt, idx + 1, item.c_str(), -1, SQLITE_TRANSIENT);
    SQL_CHECK(status == SQLITE_OK, status, sqlite3_errmsg(m_db));
}

void SQLiteStatement::sub_bindDouble(int idx, double item) {
    _reset();
    int status = sqlite3_bind_double(m_stmt, idx + 1, item);
    SQL_CHECK(status == SQLITE_OK, status, sqlite3_errmsg(m_db));
}

void SQLiteStatement::sub_bindBlob(int idx, const string& item) {
    _reset();
    int status = sqlite3_bind_blob(m_stmt, idx + 1, item.data(), item.size(), SQLITE_TRANSIENT);
    SQL_CHECK(status == SQLITE_OK, status, sqlite3_errmsg(m_db));
}

void SQLiteStatement::sub_getColumnAsInt64(int idx, int64_t& item) {
    item = sqlite3_column_int64(m_stmt, idx);
}

void SQLiteStatement::sub_getColumnAsDouble(int idx, double& item) {
    item = sqlite3_column_double(m_stmt, idx);
}

void SQLiteStatement::sub_getColumnAsText(int idx, string& item) {
    const char* data = reinterpret_cast<const char*>(sqlite3_column_text(m_stmt, idx));
    item = (data != 0) ? string(data) : string();
}

void SQLiteStatement::sub_getColumnAsBlob(int idx, string& item) {
    const char* data = static_cast<const char*>(sqlite3_column_blob(m_stmt, idx));
    if (data == NULL) {
        throw null_blob_exception();
    }
    const int size = sqlite3_column_bytes(m_stmt, idx);
    item = std::string(data, size);
}

}  // namespace hku