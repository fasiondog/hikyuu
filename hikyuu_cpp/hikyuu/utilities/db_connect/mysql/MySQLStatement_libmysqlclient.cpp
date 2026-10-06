/*
 * MySQLStatement_libmysqlclient.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#include <cstring>
#include <variant>
#include <vector>
#include "MySQLStatement.h"
#include "MySQLConnect.h"

#if defined(_MSC_VER)
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4267)
#endif

namespace hku {

constexpr unsigned long long MAX_INITIAL_BUFFER_SIZE = 1ull << 20;

// The Pimpl implementation struct
struct MySQLStatement::Impl {
    using Value = std::variant<std::monostate, int8_t, short, int32_t, int64_t, uint64_t, float,
                               double, MYSQL_TIME, std::string, std::vector<char>>;

    MYSQL* db{nullptr};
    MYSQL_STMT* stmt{nullptr};
    MYSQL_RES* meta_result{nullptr};
    bool needs_reset{false};
    bool has_bind_result{false};

    std::vector<MYSQL_BIND> param_bind;
    std::vector<MYSQL_BIND> result_bind;
    // Parameter values are stored per placeholder slot (indexed by the bind position), so the
    // storage is bounded by the parameter count no matter how many times the statement is
    // executed.
    std::vector<Value> param_buffer;
    std::vector<Value> result_buffer;
    std::vector<unsigned long> result_length;
    std::vector<char> result_is_null;
    std::vector<char> result_error;

    static bool isStringType(enum enum_field_types t) {
        return t == MYSQL_TYPE_VAR_STRING || t == MYSQL_TYPE_STRING || t == MYSQL_TYPE_BLOB ||
               t == MYSQL_TYPE_TINY_BLOB || t == MYSQL_TYPE_MEDIUM_BLOB ||
               t == MYSQL_TYPE_LONG_BLOB || t == MYSQL_TYPE_VARCHAR || t == MYSQL_TYPE_DECIMAL ||
               t == MYSQL_TYPE_NEWDECIMAL;
    }

    /** Return the value slot of a placeholder so rebinding replaces the old value instead of
     *  appending a new one (storage stays bounded by the parameter count across exec calls) */
    Value& paramSlot(int idx) {
        return param_buffer[idx];
    }

    void fetchTruncatedColumns() {
        for (size_t i = 0; i < result_bind.size(); ++i) {
            if (!result_error[i] || result_is_null[i]) {
                continue;
            }

            MYSQL_BIND& bind = result_bind[i];
            if (isStringType(bind.buffer_type) && result_length[i] > bind.buffer_length) {
                std::vector<char>* p = std::get_if<std::vector<char>>(&result_buffer[i]);
                p->resize(result_length[i] + 1);
                bind.buffer = p->data();
                bind.buffer_length = result_length[i] + 1;
                int ret = mysql_stmt_fetch_column(stmt, &bind, i, 0);
                SQL_CHECK(ret == 0, static_cast<int>(mysql_stmt_errno(stmt)),
                          "Failed fetch truncated column {}! {}", i, mysql_stmt_error(stmt));
                result_error[i] = 0;
            } else {
                SQL_THROW(MYSQL_DATA_TRUNCATED, "Data truncated in column {}!", i);
            }
        }
    }
};

MySQLStatement::MySQLStatement(DBConnectBase* driver, const std::string& sql_statement)
: SQLStatementBase(driver, sql_statement), m_impl(std::make_unique<Impl>()) {
    const MySQLConnect* connect = dynamic_cast<MySQLConnect*>(driver);
    SQL_CHECK(connect, -1, "Failed create statement: {}! Failed dynamic_cast<MySQLConnect*>!",
              sql_statement);

    m_impl->db = static_cast<MYSQL*>(connect->getRawConnection());
    _prepare();

    auto param_count = mysql_stmt_param_count(m_impl->stmt);
    if (param_count > 0) {
        m_impl->param_bind.resize(param_count);
        memset(m_impl->param_bind.data(), 0, param_count * sizeof(MYSQL_BIND));
        m_impl->param_buffer.resize(param_count);
    }

    m_impl->meta_result = mysql_stmt_result_metadata(m_impl->stmt);
    if (m_impl->meta_result) {
        int column_count = mysql_num_fields(m_impl->meta_result);
        m_impl->result_bind.resize(column_count);
        memset(m_impl->result_bind.data(), 0, column_count * sizeof(MYSQL_BIND));
        m_impl->result_buffer.resize(column_count);
        m_impl->result_length.resize(column_count, 0);
        m_impl->result_is_null.resize(column_count, 0);
        m_impl->result_error.resize(column_count, 0);
    }
}

MySQLStatement::~MySQLStatement() {
    if (m_impl->meta_result) {
        mysql_free_result(m_impl->meta_result);
    }
    if (m_impl->stmt) {
        mysql_stmt_close(m_impl->stmt);
    }
}

void MySQLStatement::_prepare() {
    m_impl->stmt = mysql_stmt_init(m_impl->db);
    HKU_CHECK(m_impl->stmt, "Failed mysql_stmt_init! SQL: {}", m_sql_string);

    int ret = mysql_stmt_prepare(m_impl->stmt, m_sql_string.c_str(), m_sql_string.size());
    HKU_IF_RETURN(0 == ret, void());

    // mysql_stmt_prepare only reports success/failure; the concrete error code must be read via
    // mysql_stmt_errno before the statement handle is closed
    int errcode = static_cast<int>(mysql_stmt_errno(m_impl->stmt));
    std::string errstr(mysql_stmt_error(m_impl->stmt));
    mysql_stmt_close(m_impl->stmt);
    m_impl->stmt = nullptr;

    // Only a lost connection is worth a reconnect attempt; other errors (e.g. a syntax error)
    // would fail the re-prepare anyway
    if (detail::isConnectionLostError(errcode)) {
        MySQLConnect* connect = dynamic_cast<MySQLConnect*>(m_driver);
        if (connect && connect->ping()) {
            m_impl->db = static_cast<MYSQL*>(connect->getRawConnection());
        } else {
            HKU_THROW("Failed reconnect mysql! SQL: {}", m_sql_string);
        }
    } else if (CR_OUT_OF_MEMORY == errcode) {
        HKU_THROW("Out of memory! SQL: {}", m_sql_string);
    }

    m_impl->stmt = mysql_stmt_init(m_impl->db);
    HKU_CHECK(m_impl->stmt, "Failed mysql_stmt_init! SQL: {}", m_sql_string);
    ret = mysql_stmt_prepare(m_impl->stmt, m_sql_string.c_str(), m_sql_string.size());
    HKU_IF_RETURN(0 == ret, void());

    errcode = static_cast<int>(mysql_stmt_errno(m_impl->stmt));
    errstr = mysql_stmt_error(m_impl->stmt);
    mysql_stmt_close(m_impl->stmt);
    m_impl->stmt = nullptr;
    HKU_THROW("Failed prepare statement: {}! errcode: {}, error msg: {}!", m_sql_string, errcode,
              errstr);
}

void MySQLStatement::_reset() {
    if (m_impl->needs_reset) {
        int ret = mysql_stmt_reset(m_impl->stmt);
        SQL_CHECK(ret == 0, static_cast<int>(mysql_stmt_errno(m_impl->stmt)),
                  "Failed reset statement! {}", mysql_stmt_error(m_impl->stmt));
        m_impl->needs_reset = false;
        m_impl->has_bind_result = false;
    }
}

void MySQLStatement::sub_exec() {
    _reset();
    m_impl->needs_reset = true;
    int ret = 0;
    if (m_impl->param_bind.size() > 0) {
        ret = mysql_stmt_bind_param(m_impl->stmt, m_impl->param_bind.data());
        SQL_CHECK(ret == 0, static_cast<int>(mysql_stmt_errno(m_impl->stmt)),
                  "Failed mysql_stmt_bind_param! {}", mysql_stmt_error(m_impl->stmt));
    }
    ret = mysql_stmt_execute(m_impl->stmt);
    SQL_CHECK(ret == 0, static_cast<int>(mysql_stmt_errno(m_impl->stmt)),
              "Failed mysql_stmt_execute: {}", mysql_stmt_error(m_impl->stmt));
}

void MySQLStatement::_bindResult() {
    HKU_IF_RETURN(!m_impl->meta_result, void());
    // Restart the field cursor so re-binding after a repeated execution walks all fields again
    mysql_field_seek(m_impl->meta_result, 0);
    MYSQL_FIELD* field;
    int idx = 0;
    while ((field = mysql_fetch_field(m_impl->meta_result))) {
        m_impl->result_bind[idx].buffer_type = field->type;
#if MYSQL_VERSION_ID >= 80000
        m_impl->result_bind[idx].is_null = (bool*)&m_impl->result_is_null[idx];
        m_impl->result_bind[idx].error = (bool*)&m_impl->result_error[idx];
#else
        m_impl->result_bind[idx].is_null = &m_impl->result_is_null[idx];
        m_impl->result_bind[idx].error = &m_impl->result_error[idx];
#endif
        m_impl->result_bind[idx].length = &m_impl->result_length[idx];

        if (field->type == MYSQL_TYPE_LONGLONG) {
            // BIGINT UNSIGNED must be read through an unsigned buffer, otherwise values above
            // INT64_MAX would be misinterpreted as negative
            if (field->flags & UNSIGNED_FLAG) {
                m_impl->result_buffer[idx] = uint64_t{0};
                m_impl->result_bind[idx].buffer =
                  std::get_if<uint64_t>(&m_impl->result_buffer[idx]);
                m_impl->result_bind[idx].is_unsigned = true;
            } else {
                m_impl->result_buffer[idx] = int64_t{0};
                m_impl->result_bind[idx].buffer = std::get_if<int64_t>(&m_impl->result_buffer[idx]);
                m_impl->result_bind[idx].is_unsigned = false;
            }
        } else if (field->type == MYSQL_TYPE_LONG || field->type == MYSQL_TYPE_INT24) {
            m_impl->result_buffer[idx] = int32_t{0};
            m_impl->result_bind[idx].buffer = std::get_if<int32_t>(&m_impl->result_buffer[idx]);
        } else if (field->type == MYSQL_TYPE_DOUBLE) {
            m_impl->result_buffer[idx] = double{0};
            m_impl->result_bind[idx].buffer = std::get_if<double>(&m_impl->result_buffer[idx]);
        } else if (field->type == MYSQL_TYPE_FLOAT) {
            m_impl->result_buffer[idx] = float{0};
            m_impl->result_bind[idx].buffer = std::get_if<float>(&m_impl->result_buffer[idx]);
        } else if (field->type == MYSQL_TYPE_VAR_STRING || field->type == MYSQL_TYPE_STRING ||
                   field->type == MYSQL_TYPE_BLOB || field->type == MYSQL_TYPE_TINY_BLOB ||
                   field->type == MYSQL_TYPE_VARCHAR || field->type == MYSQL_TYPE_DECIMAL ||
                   field->type == MYSQL_TYPE_NEWDECIMAL) {
            unsigned long long want = (unsigned long long)field->length + 1;
            unsigned long length = want > MAX_INITIAL_BUFFER_SIZE
                                     ? (unsigned long)MAX_INITIAL_BUFFER_SIZE
                                     : (unsigned long)want;
            m_impl->result_bind[idx].buffer_length = length;
            m_impl->result_buffer[idx] = std::vector<char>(length);
            m_impl->result_bind[idx].buffer =
              std::get_if<std::vector<char>>(&m_impl->result_buffer[idx])->data();
        } else if (field->type == MYSQL_TYPE_TINY) {
            m_impl->result_buffer[idx] = int8_t{0};
            m_impl->result_bind[idx].buffer = std::get_if<int8_t>(&m_impl->result_buffer[idx]);
        } else if (field->type == MYSQL_TYPE_SHORT || field->type == MYSQL_TYPE_YEAR) {
            m_impl->result_buffer[idx] = short{0};
            m_impl->result_bind[idx].buffer_type = MYSQL_TYPE_SHORT;
            m_impl->result_bind[idx].buffer = std::get_if<short>(&m_impl->result_buffer[idx]);
        } else if (field->type == MYSQL_TYPE_DATETIME || field->type == MYSQL_TYPE_DATE ||
                   field->type == MYSQL_TYPE_TIMESTAMP || field->type == MYSQL_TYPE_TIME ||
                   field->type == MYSQL_TYPE_TIME2) {
            MYSQL_TIME item;
            memset(&item, 0, sizeof(item));
            m_impl->result_buffer[idx] = item;
            m_impl->result_bind[idx].buffer = std::get_if<MYSQL_TIME>(&m_impl->result_buffer[idx]);
        } else {
            HKU_THROW("Unsupport field type: {}, field name: {}", int(field->type), field->name);
        }

        idx++;
    }
}

bool MySQLStatement::sub_moveNext() {
    int ret = 0;
    if (!m_impl->has_bind_result) {
        _bindResult();
        m_impl->has_bind_result = true;

        ret = mysql_stmt_bind_result(m_impl->stmt, m_impl->result_bind.data());
        SQL_CHECK(ret == 0, static_cast<int>(mysql_stmt_errno(m_impl->stmt)),
                  "Failed mysql_stmt_bind_result! {}", mysql_stmt_error(m_impl->stmt));

        ret = mysql_stmt_store_result(m_impl->stmt);
        SQL_CHECK(ret == 0, static_cast<int>(mysql_stmt_errno(m_impl->stmt)),
                  "Failed mysql_stmt_store_result! {}", mysql_stmt_error(m_impl->stmt));
    }

    ret = mysql_stmt_fetch(m_impl->stmt);
    if (ret == 0) {
        return true;
    } else if (ret == MYSQL_DATA_TRUNCATED) {
        m_impl->fetchTruncatedColumns();
        return true;
    } else if (ret != MYSQL_NO_DATA) {
        SQL_THROW(ret, "Error occurred in mysql_stmt_fetch! {}", mysql_stmt_error(m_impl->stmt));
    }
    return false;
}

void MySQLStatement::sub_bindNull(int idx) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_NULL;
}

void MySQLStatement::sub_bindInt(int idx, int64_t value) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    auto& buf = m_impl->paramSlot(idx);
    buf = value;
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    m_impl->param_bind[idx].is_unsigned = false;
    m_impl->param_bind[idx].buffer = std::get_if<int64_t>(&buf);
}

void MySQLStatement::sub_bindUInt64(int idx, uint64_t value) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    auto& buf = m_impl->paramSlot(idx);
    buf = value;
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    m_impl->param_bind[idx].is_unsigned = true;
    m_impl->param_bind[idx].buffer = std::get_if<uint64_t>(&buf);
}

void MySQLStatement::sub_bindDouble(int idx, double item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    auto& buf = m_impl->paramSlot(idx);
    buf = item;
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_DOUBLE;
    m_impl->param_bind[idx].buffer = std::get_if<double>(&buf);
}

void MySQLStatement::sub_bindDatetime(int idx, const Datetime& item) {
    if (item == Null<Datetime>()) {
        sub_bindNull(idx);
        return;
    }

    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    MYSQL_TIME tm;
    tm.year = static_cast<unsigned int>(item.year());
    tm.month = static_cast<unsigned int>(item.month());
    tm.day = static_cast<unsigned int>(item.day());
    tm.hour = static_cast<unsigned int>(item.hour());
    tm.minute = static_cast<unsigned int>(item.minute());
    tm.second = static_cast<unsigned int>(item.second());
    tm.second_part = static_cast<unsigned long>(item.millisecond() * 1000 + item.microsecond());
    tm.time_type = MYSQL_TIMESTAMP_DATETIME;
    auto& buf = m_impl->paramSlot(idx);
    buf = tm;
    MYSQL_TIME* p = std::get_if<MYSQL_TIME>(&buf);
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_DATETIME;
    m_impl->param_bind[idx].buffer = p;
    m_impl->param_bind[idx].buffer_length = sizeof(MYSQL_TIME);
    m_impl->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindText(int idx, const std::string& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    auto& buf = m_impl->paramSlot(idx);
    buf = item;
    std::string* p = std::get_if<std::string>(&buf);
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_VAR_STRING;
    m_impl->param_bind[idx].buffer = (void*)p->data();
    m_impl->param_bind[idx].buffer_length = item.size();
    m_impl->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindText(int idx, const char* item, size_t len) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    auto& buf = m_impl->paramSlot(idx);
    buf = std::string(item, len);
    std::string* p = std::get_if<std::string>(&buf);
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_VAR_STRING;
    m_impl->param_bind[idx].buffer = (void*)p->data();
    m_impl->param_bind[idx].buffer_length = p->size();
    m_impl->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindBlob(int idx, const std::string& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    auto& buf = m_impl->paramSlot(idx);
    buf = item;
    std::string* p = std::get_if<std::string>(&buf);
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_BLOB;
    m_impl->param_bind[idx].buffer = (void*)p->data();
    m_impl->param_bind[idx].buffer_length = item.size();
    m_impl->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindBlob(int idx, const std::vector<char>& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    auto& buf = m_impl->paramSlot(idx);
    buf = item;
    std::vector<char>* p = std::get_if<std::vector<char>>(&buf);
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_BLOB;
    m_impl->param_bind[idx].buffer = (void*)p->data();
    m_impl->param_bind[idx].buffer_length = p->size();
    m_impl->param_bind[idx].is_null = 0;
}

int MySQLStatement::sub_getNumColumns() const {
    return mysql_stmt_field_count(m_impl->stmt);
}

void MySQLStatement::sub_getColumnAsInt64(int idx, int64_t& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->result_buffer.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->result_buffer.size());

    SQL_CHECK(m_impl->result_error[idx] == 0, -1,
              "Error occurred in sub_getColumnAsint64_t! idx: {}", idx);

    if (m_impl->result_is_null[idx]) {
        item = Null<int64_t>();
        return;
    }

    try {
        if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_LONGLONG) {
            if (m_impl->result_bind[idx].is_unsigned) {
                uint64_t u = std::get<uint64_t>(m_impl->result_buffer[idx]);
                SQL_CHECK(u <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()), -1,
                          "Column {} unsigned value {} overflows int64", idx, u);
                item = static_cast<int64_t>(u);
            } else {
                item = std::get<int64_t>(m_impl->result_buffer[idx]);
            }
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_LONG ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_INT24) {
            item = std::get<int32_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_TINY) {
            item = std::get<int8_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_SHORT ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_YEAR) {
            item = std::get<short>(m_impl->result_buffer[idx]);
        } else {
            HKU_THROW("Field type mismatch! idx: {}", idx);
        }
    } catch (const hku::exception&) {
        throw;
    } catch (const std::exception& e) {
        HKU_THROW("Failed get column idx: {}! {}", idx, e.what());
    } catch (...) {
        HKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
    }
}

void MySQLStatement::sub_getColumnAsUInt64(int idx, uint64_t& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->result_buffer.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->result_buffer.size());

    SQL_CHECK(m_impl->result_error[idx] == 0, -1,
              "Error occurred in sub_getColumnAsUInt64! idx: {}", idx);

    if (m_impl->result_is_null[idx]) {
        item = (std::numeric_limits<uint64_t>::max)();
        return;
    }

    try {
        if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_LONGLONG) {
            if (m_impl->result_bind[idx].is_unsigned) {
                item = std::get<uint64_t>(m_impl->result_buffer[idx]);
            } else {
                int64_t s = std::get<int64_t>(m_impl->result_buffer[idx]);
                SQL_CHECK(s >= 0, -1, "Column {} holds negative value {}, cannot be read as uint64",
                          idx, s);
                item = static_cast<uint64_t>(s);
            }
        } else {
            int64_t s = 0;
            sub_getColumnAsInt64(idx, s);
            SQL_CHECK(s >= 0, -1, "Column {} holds negative value {}, cannot be read as uint64",
                      idx, s);
            item = static_cast<uint64_t>(s);
        }
    } catch (const hku::exception&) {
        throw;
    } catch (const std::exception& e) {
        HKU_THROW("Failed get column idx: {}! {}", idx, e.what());
    } catch (...) {
        HKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
    }
}

void MySQLStatement::sub_getColumnAsDouble(int idx, double& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->result_buffer.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->result_buffer.size());

    SQL_CHECK(m_impl->result_error[idx] == 0, -1,
              "Error occurred in sub_getColumnAsDouble! idx: {}", idx);

    if (m_impl->result_is_null[idx]) {
        item = Null<double>();
        return;
    }

    try {
        if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_DOUBLE) {
            item = std::get<double>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_FLOAT) {
            item = std::get<float>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_LONGLONG) {
            // BIGINT is stored in the variant as uint64_t or int64_t depending on the column's
            // unsigned flag; pick the active alternative, otherwise reading an unsigned BIGINT as
            // double throws bad_variant_access (mirrors sub_getColumnAsInt64/UInt64)
            if (m_impl->result_bind[idx].is_unsigned) {
                item = std::get<uint64_t>(m_impl->result_buffer[idx]);
            } else {
                item = std::get<int64_t>(m_impl->result_buffer[idx]);
            }
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_LONG ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_INT24) {
            item = std::get<int32_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_TINY) {
            item = std::get<int8_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_SHORT ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_YEAR) {
            item = std::get<short>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_DECIMAL ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_NEWDECIMAL) {
            std::vector<char>* p = std::get_if<std::vector<char>>(&(m_impl->result_buffer[idx]));
            SQL_CHECK(m_impl->result_length[idx] <= p->size(), -1, "Invalid column length! idx: {}",
                      idx);
            item = std::stod(std::string(p->data(), m_impl->result_length[idx]));
        } else {
            HKU_THROW("Field type({}) mismatch! idx: {}", int(m_impl->result_bind[idx].buffer_type),
                      idx);
        }
    } catch (const hku::exception&) {
        throw;
    } catch (const std::exception& e) {
        HKU_THROW("Failed get column idx: {}! {}", idx, e.what());
    } catch (...) {
        HKU_THROW("Failed get columon idx: {}! Unknown error!", idx);
    }
}

void MySQLStatement::sub_getColumnAsDatetime(int idx, Datetime& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->result_buffer.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->result_buffer.size());

    SQL_CHECK(m_impl->result_error[idx] == 0, -1,
              "Error occurred in sub_getColumnAsDatetime! idx: {}", idx);

    if (m_impl->result_is_null[idx]) {
        item = Null<Datetime>();
        return;
    }

    try {
        const MYSQL_TIME* tm = std::get_if<MYSQL_TIME>(&(m_impl->result_buffer[idx]));
        if (tm->time_type == MYSQL_TIMESTAMP_DATETIME) {
            long millisec = tm->second_part / 1000;
            long microsec = tm->second_part - millisec * 1000;
            item = Datetime(tm->year, tm->month, tm->day, tm->hour, tm->minute, tm->second,
                            millisec, microsec);
        } else if (tm->time_type == MYSQL_TIMESTAMP_DATE) {
            item = Datetime(tm->year, tm->month, tm->day);
        } else {
            HKU_THROW("Unsupported type: {}, Field type mismatch! idx: {}",
                      int(m_impl->result_bind[idx].buffer_type), idx);
        }
    } catch (const hku::exception&) {
        throw;
    } catch (...) {
        HKU_THROW("Field type mismatch! idx: {}", idx);
    }
}

void MySQLStatement::sub_getColumnAsText(int idx, std::string& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->result_buffer.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->result_buffer.size());

    SQL_CHECK(m_impl->result_error[idx] == 0, -1, "Error occurred in sub_getColumnAsText! idx: {}",
              idx);

    if (m_impl->result_is_null[idx]) {
        item.clear();
        return;
    }

    try {
        if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_DATETIME ||
            m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_TIMESTAMP ||
            m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_DATE ||
            m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_TIME) {
            const MYSQL_TIME* tm = std::get_if<MYSQL_TIME>(&(m_impl->result_buffer[idx]));
            if (tm->time_type == MYSQL_TIMESTAMP_DATETIME) {
                long millisec = tm->second_part / 1000;
                long microsec = tm->second_part - millisec * 1000;
                item = Datetime(tm->year, tm->month, tm->day, tm->hour, tm->minute, tm->second,
                                millisec, microsec)
                         .str();
            } else if (tm->time_type == MYSQL_TIMESTAMP_DATE) {
                item = Datetime(tm->year, tm->month, tm->day).str();
            } else if (tm->time_type == MYSQL_TIMESTAMP_TIME) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%02d:%02d:%02d", tm->hour, tm->minute, tm->second);
                item = std::string(buf);
            } else {
                HKU_THROW("Unsupported type: {}, Field type mismatch! idx: {}",
                          int(m_impl->result_bind[idx].buffer_type), idx);
            }
            return;
        }

        std::vector<char>* p = std::get_if<std::vector<char>>(&(m_impl->result_buffer[idx]));
        SQL_CHECK(m_impl->result_length[idx] <= p->size(), -1, "Invalid column length! idx: {}",
                  idx);
        item.assign(p->data(), m_impl->result_length[idx]);
    } catch (...) {
        HKU_THROW("Field type mismatch! idx: {}", idx);
    }
}

void MySQLStatement::sub_getColumnAsBlob(int idx, std::string& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->result_buffer.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->result_buffer.size());

    SQL_CHECK(m_impl->result_error[idx] == 0, -1, "Error occurred in sub_getColumnAsBlob! idx: {}",
              idx);

    if (m_impl->result_is_null[idx]) {
        throw null_blob_exception();
    }

    try {
        std::vector<char>* p = std::get_if<std::vector<char>>(&m_impl->result_buffer[idx]);
        SQL_CHECK(m_impl->result_length[idx] <= p->size(), -1, "Invalid column length! idx: {}",
                  idx);
        item.assign(p->data(), m_impl->result_length[idx]);
    } catch (...) {
        HKU_THROW("Field type mismatch! idx: {}", idx);
    }
}

void MySQLStatement::sub_getColumnAsBlob(int idx, std::vector<char>& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->result_buffer.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->result_buffer.size());

    SQL_CHECK(m_impl->result_error[idx] == 0, -1, "Error occurred in sub_getColumnAsBlob! idx: {}",
              idx);

    if (m_impl->result_is_null[idx]) {
        throw null_blob_exception();
    }

    try {
        unsigned long len = m_impl->result_length[idx];
        std::vector<char>* p = std::get_if<std::vector<char>>(&m_impl->result_buffer[idx]);
        item.resize(len);
        memcpy(item.data(), p->data(), len);

    } catch (...) {
        HKU_THROW("Field type mismatch! idx: {}", idx);
    }
}

uint64_t MySQLStatement::sub_getLastRowid() {
    return mysql_stmt_insert_id(m_impl->stmt);
}

}  // namespace hku

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
