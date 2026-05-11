/*
 * MySQLStatement_libmysqlclient.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

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

// Pimpl 实现结构体
struct MySQLStatement::Impl {
    MYSQL* db{nullptr};
    MYSQL_STMT* stmt{nullptr};
    MYSQL_RES* meta_result{nullptr};
    bool needs_reset{false};
    bool has_bind_result{false};

    std::vector<MYSQL_BIND> param_bind;
    std::vector<MYSQL_BIND> result_bind;
    std::vector<boost::any> param_buffer;
    std::vector<boost::any> result_buffer;
    std::vector<unsigned long> result_length;
    std::vector<char> result_is_null;
    std::vector<char> result_error;
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
    }

    m_impl->meta_result = mysql_stmt_result_metadata(m_impl->stmt);
    if (m_impl->meta_result) {
        int column_count = mysql_num_fields(m_impl->meta_result);
        m_impl->result_bind.resize(column_count);
        memset(m_impl->result_bind.data(), 0, column_count * sizeof(MYSQL_BIND));
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

    mysql_stmt_close(m_impl->stmt);
    m_impl->stmt = nullptr;

    // 如果是服务器异常，尝试重连服务器
    // 1 是 Lost connection to MySQL server during query，但 MYSQL 没有错误码定义
    if (1 == ret || CR_SERVER_LOST == ret || CR_SERVER_GONE_ERROR == ret) {
        MySQLConnect* connect = dynamic_cast<MySQLConnect*>(m_driver);
        if (connect && connect->ping()) {
            m_impl->db = static_cast<MYSQL*>(connect->getRawConnection());
        } else {
            HKU_THROW("Failed reconnect mysql! SQL: {}", m_sql_string);
        }
    } else if (CR_OUT_OF_MEMORY == ret) {
        HKU_THROW("Out of memory! SQL: {}", m_sql_string);
    }

    m_impl->stmt = mysql_stmt_init(m_impl->db);
    ret = mysql_stmt_prepare(m_impl->stmt, m_sql_string.c_str(), m_sql_string.size());
    HKU_IF_RETURN(0 == ret, void());

    std::string stmt_errorstr(mysql_stmt_error(m_impl->stmt));
    mysql_stmt_close(m_impl->stmt);
    m_impl->stmt = nullptr;
    HKU_THROW("Failed prepare statement: {}! ret: {}, error msg: {}!", m_sql_string, ret,
              stmt_errorstr);
}

void MySQLStatement::_reset() {
    if (m_impl->needs_reset) {
        int ret = mysql_stmt_reset(m_impl->stmt);
        SQL_CHECK(ret == 0, ret, "Failed reset statement! {}", mysql_stmt_error(m_impl->stmt));
        m_impl->result_buffer.clear();
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
        SQL_CHECK(ret == 0, ret, "Failed mysql_stmt_bind_param! {}",
                  mysql_stmt_error(m_impl->stmt));
    }
    ret = mysql_stmt_execute(m_impl->stmt);
    SQL_CHECK(ret == 0, ret, "Failed mysql_stmt_execute: {}", mysql_stmt_error(m_impl->stmt));
}

void MySQLStatement::_bindResult() {
    HKU_IF_RETURN(!m_impl->meta_result, void());
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
            int64_t item = 0;
            m_impl->result_buffer.push_back(item);
            auto& buf = m_impl->result_buffer.back();
            m_impl->result_bind[idx].buffer = boost::any_cast<int64_t>(&buf);
        } else if (field->type == MYSQL_TYPE_LONG || field->type == MYSQL_TYPE_INT24) {
            int32_t item = 0;
            m_impl->result_buffer.push_back(item);
            auto& buf = m_impl->result_buffer.back();
            m_impl->result_bind[idx].buffer = boost::any_cast<int32_t>(&buf);
        } else if (field->type == MYSQL_TYPE_DOUBLE) {
            double item = 0;
            m_impl->result_buffer.push_back(item);
            auto& buf = m_impl->result_buffer.back();
            m_impl->result_bind[idx].buffer = boost::any_cast<double>(&buf);
        } else if (field->type == MYSQL_TYPE_FLOAT) {
            float item = 0;
            m_impl->result_buffer.push_back(item);
            auto& buf = m_impl->result_buffer.back();
            m_impl->result_bind[idx].buffer = boost::any_cast<float>(&buf);
        } else if (field->type == MYSQL_TYPE_VAR_STRING || field->type == MYSQL_TYPE_STRING ||
                   field->type == MYSQL_TYPE_BLOB || field->type == MYSQL_TYPE_TINY_BLOB ||
                   field->type == MYSQL_TYPE_VARCHAR || field->type == MYSQL_TYPE_DECIMAL ||
                   field->type == MYSQL_TYPE_NEWDECIMAL) {
            unsigned long length = field->length + 1;
            m_impl->result_bind[idx].buffer_length = length;
            m_impl->result_buffer.emplace_back(std::vector<char>(length));
            auto& buf = m_impl->result_buffer.back();
            std::vector<char>* p = boost::any_cast<std::vector<char>>(&buf);
            m_impl->result_bind[idx].buffer = p->data();
        } else if (field->type == MYSQL_TYPE_TINY) {
            int8_t item = 0;
            m_impl->result_buffer.push_back(item);
            auto& buf = m_impl->result_buffer.back();
            m_impl->result_bind[idx].buffer = boost::any_cast<int8_t>(&buf);
        } else if (field->type == MYSQL_TYPE_SHORT || field->type == MYSQL_TYPE_YEAR) {
            short item = 0;
            m_impl->result_buffer.push_back(item);
            auto& buf = m_impl->result_buffer.back();
            m_impl->result_bind[idx].buffer_type = MYSQL_TYPE_SHORT;
            m_impl->result_bind[idx].buffer = boost::any_cast<short>(&buf);
        } else if (field->type == MYSQL_TYPE_DATETIME || field->type == MYSQL_TYPE_DATE ||
                   field->type == MYSQL_TYPE_TIMESTAMP || field->type == MYSQL_TYPE_TIME ||
                   field->type == MYSQL_TYPE_TIME2) {
            MYSQL_TIME item;
            std::memset(&item, 0, sizeof(item));
            m_impl->result_buffer.push_back(item);
            auto& buf = m_impl->result_buffer.back();
            m_impl->result_bind[idx].buffer = boost::any_cast<MYSQL_TIME>(&buf);
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
        SQL_CHECK(ret == 0, ret, "Failed mysql_stmt_bind_result! {}",
                  mysql_stmt_error(m_impl->stmt));

        ret = mysql_stmt_store_result(m_impl->stmt);
        SQL_CHECK(ret == 0, ret, "Failed mysql_stmt_store_result! {}",
                  mysql_stmt_error(m_impl->stmt));
    }

    ret = mysql_stmt_fetch(m_impl->stmt);
    if (ret == 0) {
        return true;
    } else if (ret == 1) {
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
    m_impl->param_buffer.push_back(value);
    auto& buf = m_impl->param_buffer.back();
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    m_impl->param_bind[idx].buffer = boost::any_cast<int64_t>(&buf);
}

void MySQLStatement::sub_bindDouble(int idx, double item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    m_impl->param_buffer.push_back(item);
    auto& buf = m_impl->param_buffer.back();
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_DOUBLE;
    m_impl->param_bind[idx].buffer = boost::any_cast<double>(&buf);
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
    m_impl->param_buffer.push_back(tm);
    auto& buf = m_impl->param_buffer.back();
    MYSQL_TIME* p = boost::any_cast<MYSQL_TIME>(&buf);
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_DATETIME;
    m_impl->param_bind[idx].buffer = p;
    m_impl->param_bind[idx].buffer_length = sizeof(MYSQL_TIME);
    m_impl->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindText(int idx, const std::string& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    m_impl->param_buffer.push_back(item);
    auto& buf = m_impl->param_buffer.back();
    std::string* p = boost::any_cast<std::string>(&buf);
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_VAR_STRING;
    m_impl->param_bind[idx].buffer = (void*)p->data();
    m_impl->param_bind[idx].buffer_length = item.size();
    m_impl->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindText(int idx, const char* item, size_t len) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    m_impl->param_buffer.push_back(std::string(item));
    auto& buf = m_impl->param_buffer.back();
    std::string* p = boost::any_cast<std::string>(&buf);
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_VAR_STRING;
    m_impl->param_bind[idx].buffer = (void*)p->data();
    m_impl->param_bind[idx].buffer_length = p->size();
    m_impl->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindBlob(int idx, const std::string& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    m_impl->param_buffer.push_back(item);
    auto& buf = m_impl->param_buffer.back();
    std::string* p = boost::any_cast<std::string>(&buf);
    m_impl->param_bind[idx].buffer_type = MYSQL_TYPE_BLOB;
    m_impl->param_bind[idx].buffer = (void*)p->data();
    m_impl->param_bind[idx].buffer_length = item.size();
    m_impl->param_bind[idx].is_null = 0;
}

void MySQLStatement::sub_bindBlob(int idx, const std::vector<char>& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->param_bind.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->param_bind.size());
    m_impl->param_buffer.push_back(item);
    auto& buf = m_impl->param_buffer.back();
    std::vector<char>* p = boost::any_cast<std::vector<char>>(&buf);
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
        item = 0;
        return;
    }

    try {
        if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_LONGLONG) {
            item = boost::any_cast<int64_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_LONG ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_INT24) {
            item = boost::any_cast<int32_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_TINY) {
            item = boost::any_cast<int8_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_SHORT ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_YEAR) {
            item = boost::any_cast<short>(m_impl->result_buffer[idx]);
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

void MySQLStatement::sub_getColumnAsDouble(int idx, double& item) {
    SQL_CHECK(idx < static_cast<int>(m_impl->result_buffer.size()), -1,
              "idx out of range! idx: {}, total: {}", idx, m_impl->result_buffer.size());

    SQL_CHECK(m_impl->result_error[idx] == 0, -1,
              "Error occurred in sub_getColumnAsDouble! idx: {}", idx);

    if (m_impl->result_is_null[idx]) {
        item = 0.0;
        return;
    }

    try {
        if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_DOUBLE) {
            item = boost::any_cast<double>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_FLOAT) {
            item = boost::any_cast<float>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_LONGLONG) {
            item = boost::any_cast<int64_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_LONG ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_INT24) {
            item = boost::any_cast<int32_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_TINY) {
            item = boost::any_cast<int8_t>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_SHORT ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_YEAR) {
            item = boost::any_cast<short>(m_impl->result_buffer[idx]);
        } else if (m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_DECIMAL ||
                   m_impl->result_bind[idx].buffer_type == MYSQL_TYPE_NEWDECIMAL) {
            std::vector<char>* p =
              boost::any_cast<std::vector<char>>(&(m_impl->result_buffer[idx]));
            std::ostringstream buf;
            for (unsigned long i = 0; i < m_impl->result_length[idx]; i++) {
                buf << (*p)[i];
            }
            item = std::stod(buf.str());
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
        const MYSQL_TIME* tm = boost::any_cast<MYSQL_TIME>(&(m_impl->result_buffer[idx]));
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
            const MYSQL_TIME* tm = boost::any_cast<MYSQL_TIME>(&(m_impl->result_buffer[idx]));
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

        std::vector<char>* p = boost::any_cast<std::vector<char>>(&(m_impl->result_buffer[idx]));
        std::ostringstream buf;
        for (unsigned long i = 0; i < m_impl->result_length[idx]; i++) {
            buf << (*p)[i];
        }
        item = buf.str();
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
        item.clear();
        return;
    }

    try {
        std::vector<char>* p = boost::any_cast<std::vector<char>>(&m_impl->result_buffer[idx]);
        std::ostringstream buf;
        for (unsigned long i = 0; i < m_impl->result_length[idx]; i++) {
            buf << (*p)[i];
        }
        item = buf.str();
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
        item.clear();
        return;
    }

    try {
        unsigned long len = m_impl->result_length[idx];
        std::vector<char>* p = boost::any_cast<std::vector<char>>(&m_impl->result_buffer[idx]);
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
