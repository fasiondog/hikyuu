/*
 * MySQLStatement.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#include <vector>
#include <chrono>
#include <boost/mysql.hpp>
#include "hikyuu/utilities/Log.h"
#include "MySQLStatement.h"
#include "MySQLConnect.h"

namespace hku {

struct MySQLStatement::Impl {
    MySQLConnect* connect{nullptr};
    std::shared_ptr<boost::mysql::statement> stmt;
    boost::mysql::results results;
    std::vector<boost::mysql::field> params;
    size_t current_row{0};
    bool has_result{false};
    bool needs_reset{false};
};

MySQLStatement::MySQLStatement(DBConnectBase* driver, const std::string& sql_statement)
: SQLStatementBase(driver, sql_statement), m_impl(std::make_unique<Impl>()) {
    m_impl->connect = dynamic_cast<MySQLConnect*>(driver);
    SQL_CHECK(m_impl->connect, -1,
              "Failed create statement: {}! Failed dynamic_cast<MySQLConnect*>!", sql_statement);
    _prepare();
}

MySQLStatement::~MySQLStatement() {
    // The statement of boost.mysql is cleaned up automatically
}

void MySQLStatement::_prepare() {
    try {
        boost::mysql::error_code ec;
        boost::mysql::diagnostics diag;
        _reset();

        // Use the get_statement method in the impl of MySQLConnect
        m_impl->stmt = m_impl->connect->m_impl->get_statement(m_sql_string, ec, diag);
        m_impl->needs_reset = true;

        if (ec) [[unlikely]] {
            _reset();

            // Judge whether it is a connection layer error (a reconnection is needed) rather than
            // an SQL application layer error The MySQL client connection error codes:
            // - 2013 (CR_SERVER_LOST): the connection is lost during the query
            // - 2006 (CR_SERVER_GONE_ERROR): the server has gone away
            // - 2003 (CR_CONN_HOST_ERROR): unable to connect to the server
            // - 2002 (CR_CONNECTION_ERROR): the local connection failed
            // - 2005 (CR_UNKNOWN_HOST): an unknown host
            bool is_connection_error = false;

            // Judge it by the error code (Boost.MySQL uses boost::system::error_code)
            int error_value = ec.value();

            // The range of the MySQL client errors is 2000-2999
            if (error_value >= 2000 && error_value <= 2999) {
                // The common connection related error codes
                switch (error_value) {
                    case 2002:  // CR_CONNECTION_ERROR
                    case 2003:  // CR_CONN_HOST_ERROR
                    case 2005:  // CR_UNKNOWN_HOST
                    case 2006:  // CR_SERVER_GONE_ERROR
                    case 2013:  // CR_SERVER_LOST
                        is_connection_error = true;
                        break;
                    default:
                        // For the other client errors, check whether the message contains a
                        // connection related keyword
                        if (!diag.server_message().empty()) {
                            const auto& msg = diag.server_message();
                            is_connection_error =
                              (msg.find("Lost connection") != std::string::npos ||
                               msg.find("gone away") != std::string::npos);
                        } else if (!diag.client_message().empty()) {
                            const auto& msg = diag.client_message();
                            is_connection_error = (msg.find("connection") != std::string::npos ||
                                                   msg.find("timeout") != std::string::npos);
                        }
                        break;
                }
            }

            // Try to reconnect only on a connection layer error
            if (is_connection_error) {
                _reset();
                if (m_impl->connect->ping()) {
                    // The ping succeeded (it reconnected automatically), get the statement again
                    m_impl->stmt = m_impl->connect->m_impl->get_statement(m_sql_string, ec, diag);
                    m_impl->needs_reset = true;

                    if (ec) [[unlikely]] {
                        // It still fails after the reconnection, print the error log
                        HKU_ERROR("Failed prepare statement after reconnect! Error code {}: {}",
                                  ec.value(), ec.message());
                        SQL_THROW(ec.value(), "Failed prepare statement after reconnect!");
                    }
                    return;
                }
            }

            // Not a connection error or the reconnection failed, throw the original error directly
            SQL_THROW(ec.value(), "Failed prepare statement! {}", m_sql_string);
        }

        HKU_ASSERT(m_impl->stmt);

    } catch (const hku::exception&) {
        throw;
    } catch (const std::exception& e) {
        SQL_THROW(-1, "Failed prepare statement: {}! {}", m_sql_string, e.what());
    } catch (...) {
        SQL_THROW(-1, "Failed prepare statement: {}! Unknown error!", m_sql_string);
    }
}

void MySQLStatement::_reset() {
    if (m_impl->needs_reset) {
        m_impl->stmt.reset();
        m_impl->results = {};
        m_impl->params.clear();
        m_impl->params.shrink_to_fit();
        m_impl->current_row = 0;
        m_impl->has_result = false;
        m_impl->needs_reset = false;
    }
}

void MySQLStatement::sub_exec() {
    boost::mysql::error_code ec;
    boost::mysql::diagnostics diag;

    // Get the underlying connection for the execution
    auto* conn = static_cast<boost::mysql::tcp_connection*>(m_impl->connect->getRawConnection());

    if (m_impl->params.empty()) {
        // Without parameters, execute directly
        conn->execute(m_sql_string, m_impl->results, ec, diag);
    } else {
        // With parameters, use the prepared statement (a field_view iterator is used uniformly)
        std::vector<boost::mysql::field_view> param_views;
        param_views.reserve(m_impl->params.size());
        for (const auto& f : m_impl->params) {
            param_views.push_back(boost::mysql::field_view(f));
        }
        auto bound = m_impl->stmt->bind(param_views.begin(), param_views.end());
        conn->execute(bound, m_impl->results, ec, diag);
    }

    if (ec) [[unlikely]] {
        SQL_THROW(ec.value(), "Failed execute sql: {}! {}", m_sql_string, ec.message());
    }

    m_impl->has_result = true;
    m_impl->needs_reset = true;
}

bool MySQLStatement::sub_moveNext() {
    if (!m_impl->has_result) {
        _reset();
        return false;
    }

    const auto& rows = m_impl->results.rows();
    if (m_impl->current_row >= rows.size()) {
        _reset();
        return false;
    }

    m_impl->current_row++;
    return true;
}

void MySQLStatement::sub_bindNull(int idx) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    m_impl->params.push_back(boost::mysql::field());  // Constructed as NULL by default
}

void MySQLStatement::sub_bindInt(int idx, int64_t value) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    m_impl->params.push_back(boost::mysql::field(static_cast<std::int64_t>(value)));
}

void MySQLStatement::sub_bindDouble(int idx, double item) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    m_impl->params.push_back(boost::mysql::field(item));
}

void MySQLStatement::sub_bindDatetime(int idx, const Datetime& item) {
    if (item == Null<Datetime>()) {
        sub_bindNull(idx);
        return;
    }

    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);

    // Use the native datetime type of boost.mysql
    boost::mysql::datetime dt(
      static_cast<std::uint16_t>(item.year()), static_cast<std::uint8_t>(item.month()),
      static_cast<std::uint8_t>(item.day()), static_cast<std::uint8_t>(item.hour()),
      static_cast<std::uint8_t>(item.minute()), static_cast<std::uint8_t>(item.second()),
      static_cast<std::uint32_t>(item.millisecond() * 1000 + item.microsecond()));
    m_impl->params.push_back(boost::mysql::field(dt));
}

void MySQLStatement::sub_bindText(int idx, const std::string& item) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    m_impl->params.push_back(boost::mysql::field(item));
}

void MySQLStatement::sub_bindText(int idx, const char* item, size_t len) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    std::string str(item, len);
    m_impl->params.push_back(boost::mysql::field(str));
}

void MySQLStatement::sub_bindBlob(int idx, const std::string& item) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    std::vector<unsigned char> blob(item.begin(), item.end());
    m_impl->params.push_back(boost::mysql::field(blob));
}

void MySQLStatement::sub_bindBlob(int idx, const std::vector<char>& item) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    std::vector<unsigned char> blob(item.begin(), item.end());
    m_impl->params.push_back(boost::mysql::field(blob));
}

int MySQLStatement::sub_getNumColumns() const {
    HKU_IF_RETURN(!m_impl->has_result, 0);
    const auto& metadata = m_impl->results.meta();
    HKU_IF_RETURN(metadata.empty(), 0);
    return static_cast<int>(metadata.size());
}

void MySQLStatement::sub_getColumnAsInt64(int idx, int64_t& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& rows = m_impl->results.rows();
    SQL_CHECK(m_impl->current_row > 0 && m_impl->current_row <= rows.size(), -1,
              "Invalid row index!");

    const auto& row = rows[m_impl->current_row - 1];
    SQL_CHECK(idx < static_cast<int>(row.size()), -1, "Column index out of range!");

    const auto& value = row[idx];
    if (value.is_null()) {
        item = 0;
        return;
    }

    try {
        // Try to convert it into int64 directly
        item = value.as_int64();
    } catch (...) {
        try {
            // Try to convert it as uint64 (YEAR may be returned as uint64)
            uint64_t u = value.as_uint64();
            item = static_cast<int64_t>(u);
        } catch (...) {
            try {
                // Finally try to parse it as a string
                std::string str = value.as_string();
                item = std::stoll(str);
            } catch (const std::exception& e) {
                SQL_THROW(-1, "Failed to convert column {} to int64: {}", idx, e.what());
            }
        }
    }
}

void MySQLStatement::sub_getColumnAsDouble(int idx, double& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& rows = m_impl->results.rows();
    SQL_CHECK(m_impl->current_row > 0 && m_impl->current_row <= rows.size(), -1,
              "Invalid row index!");

    const auto& row = rows[m_impl->current_row - 1];
    SQL_CHECK(idx < static_cast<int>(row.size()), -1, "Column index out of range!");

    const auto& value = row[idx];
    if (value.is_null()) {
        item = 0.0;
        return;
    }

    try {
        // Try to convert it into double directly
        item = value.as_double();
    } catch (...) {
        try {
            // If it fails, try to convert it as float
            float f = value.as_float();
            item = static_cast<double>(f);
        } catch (...) {
            try {
                // Finally try to parse it as a string (the DECIMAL type may be returned as a
                // string)
                std::string str = value.as_string();
                item = std::stod(str);
            } catch (const std::exception& e) {
                SQL_THROW(-1, "Failed to convert column {} to double: {}", idx, e.what());
            }
        }
    }
}

void MySQLStatement::sub_getColumnAsDatetime(int idx, Datetime& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& rows = m_impl->results.rows();
    SQL_CHECK(m_impl->current_row > 0 && m_impl->current_row <= rows.size(), -1,
              "Invalid row index!");

    const auto& row = rows[m_impl->current_row - 1];
    SQL_CHECK(idx < static_cast<int>(row.size()), -1, "Column index out of range!");

    const auto& value = row[idx];
    if (value.is_null()) {
        item = Null<Datetime>();
        return;
    }

    try {
        // Try to read it as datetime first
        auto dt = value.as_datetime();
        item = Datetime(
          static_cast<long>(dt.year()), static_cast<long>(dt.month()), static_cast<long>(dt.day()),
          static_cast<long>(dt.hour()), static_cast<long>(dt.minute()),
          static_cast<long>(dt.second()),
          static_cast<long>(dt.microsecond() / 1000),   // Convert the microseconds to milliseconds
          static_cast<long>(dt.microsecond() % 1000));  // The remaining microseconds
    } catch (...) {
        try {
            // Try to read it as date (there is no time part)
            auto d = value.as_date();
            item = Datetime(static_cast<long>(d.year()), static_cast<long>(d.month()),
                            static_cast<long>(d.day()));
        } catch (...) {
            // Finally try to parse it as a string
            std::string datetime_str = value.as_string();
            item = Datetime(datetime_str);
        }
    }
}

void MySQLStatement::sub_getColumnAsText(int idx, std::string& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& rows = m_impl->results.rows();
    SQL_CHECK(m_impl->current_row > 0 && m_impl->current_row <= rows.size(), -1,
              "Invalid row index!");

    const auto& row = rows[m_impl->current_row - 1];
    SQL_CHECK(idx < static_cast<int>(row.size()), -1, "Column index out of range!");

    const auto& value = row[idx];
    if (value.is_null()) {
        item.clear();
        return;
    }

    try {
        // Try to read it as a string directly
        item = value.as_string();
    } catch (...) {
        // If it fails it may be a date time type and needs a special handling
        try {
            // Try to read it as datetime
            auto dt = value.as_datetime();
            // Format it as a string: YYYY-MM-DD HH:MM:SS
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d", dt.year(), dt.month(),
                     dt.day(), dt.hour(), dt.minute(), dt.second());
            item = buffer;
        } catch (...) {
            try {
                // Try to read it as date
                auto d = value.as_date();
                // Format it as a string: YYYY-MM-DD
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", d.year(), d.month(), d.day());
                item = buffer;
            } catch (...) {
                try {
                    // Finally try to read it as time (boost::mysql::time is a duration type)
                    auto t = value.as_time();
                    // Convert the duration into hours, minutes and seconds
                    auto total_seconds =
                      std::chrono::duration_cast<std::chrono::seconds>(t).count();
                    int hours = total_seconds / 3600;
                    int minutes = (total_seconds % 3600) / 60;
                    int seconds = total_seconds % 60;
                    // Format it as a string: HH:MM:SS
                    char buffer[32];
                    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
                    item = buffer;
                } catch (...) {
                    SQL_THROW(-1, "Failed to convert column {} to string", idx);
                }
            }
        }
    }
}

void MySQLStatement::sub_getColumnAsBlob(int idx, std::string& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& rows = m_impl->results.rows();
    SQL_CHECK(m_impl->current_row > 0 && m_impl->current_row <= rows.size(), -1,
              "Invalid row index!");

    const auto& row = rows[m_impl->current_row - 1];
    SQL_CHECK(idx < static_cast<int>(row.size()), -1, "Column index out of range!");

    const auto& value = row[idx];
    if (value.is_null()) {
        item.clear();
        return;
    }

    try {
        auto blob = value.as_blob();
        item.assign(blob.begin(), blob.end());
    } catch (...) {
        // If it is not a blob type, try to read it as a string
        item = value.as_string();
    }
}

void MySQLStatement::sub_getColumnAsBlob(int idx, std::vector<char>& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& rows = m_impl->results.rows();
    SQL_CHECK(m_impl->current_row > 0 && m_impl->current_row <= rows.size(), -1,
              "Invalid row index!");

    const auto& row = rows[m_impl->current_row - 1];
    SQL_CHECK(idx < static_cast<int>(row.size()), -1, "Column index out of range!");

    const auto& value = row[idx];
    if (value.is_null()) {
        item.clear();
        return;
    }

    try {
        const auto& blob = value.as_blob();
        item.assign(blob.begin(), blob.end());
    } catch (...) {
        // If it is not a blob type, try to read it as a string
        std::string str = value.as_string();
        item.assign(str.begin(), str.end());
    }
}

uint64_t MySQLStatement::sub_getLastRowid() {
    return m_impl->results.last_insert_id();
}

}  // namespace hku