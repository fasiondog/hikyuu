/*
 * AsyncMySQLStatement.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */

#include <vector>
#include <chrono>
#include <boost/mysql.hpp>
#include "hikyuu/utilities/Log.h"
#include "AsyncMySQLStatement.h"
#include "AsyncMySQLConnect.h"

namespace hku {

struct AsyncMySQLStatement::Impl {
    AsyncMySQLConnect* connect{nullptr};
    std::shared_ptr<boost::mysql::statement> stmt;
    boost::mysql::results results;             // Used by async_execute (the case with parameters)
    boost::mysql::execution_state exec_state;  // The state used for the streaming reading
    std::vector<boost::mysql::row> current_batch;  // The data of the current batch (owned)
    std::vector<boost::mysql::field> params;
    size_t current_row{0};      // The row index within the current batch
    size_t total_rows_read{0};  // The total number of the rows read
    bool has_result{false};
    bool needs_reset{false};
    bool is_streaming{false};  // Whether the streaming mode is used

    // Helper function: get the field_view of the current row
    boost::mysql::field_view getField(int idx) const {
        if (is_streaming) {
            // The streaming mode: get it from current_batch
            SQL_CHECK(current_row > 0 && current_row <= current_batch.size(), -1,
                      "Invalid row index in streaming mode!");
            const auto& row = current_batch[current_row - 1];
            SQL_CHECK(idx < static_cast<int>(row.size()), -1, "Column index out of range!");
            return row[idx];
        } else {
            // The non-streaming mode: get it from results
            const auto& rows = results.rows();
            SQL_CHECK(current_row > 0 && current_row <= rows.size(), -1, "Invalid row index!");
            const auto& row = *(rows.begin() + (current_row - 1));
            SQL_CHECK(idx < static_cast<int>(row.size()), -1, "Column index out of range!");
            return row[idx];
        }
    }
};

AsyncMySQLStatement::AsyncMySQLStatement(AsyncMySQLConnect* connect, const std::string& sql)
: AsyncSQLStatementBase(connect, sql), m_impl(std::make_unique<Impl>()) {
    m_impl->connect = connect;
    SQL_CHECK(m_impl->connect, -1,
              "Failed create statement: {}! Failed dynamic_cast<AsyncMySQLConnect*>!", sql);
}

AsyncMySQLStatement::~AsyncMySQLStatement() {
    // The statement of boost.mysql is cleaned up automatically
}

net::awaitable<void> AsyncMySQLStatement::sub_exec() {
    // Prepare the statement when there is no prepared statement yet
    if (!m_impl->stmt) {
        boost::mysql::error_code ec;
        boost::mysql::diagnostics diag;

        // An asynchronous connection cannot connect actively at the construction, so the connection
        // must be established first here
        if (!m_impl->connect->m_impl->initialized) {
            co_await m_impl->connect->connect();
        }

        m_impl->stmt = co_await m_impl->connect->m_impl->get_statement(m_sql_string, ec, diag);
        m_impl->needs_reset = true;

        if (ec) {
            // Judge whether it is a connection layer error
            int error_value = ec.value();
            bool is_connection_error = false;

            if (error_value >= 2000 && error_value <= 2999) {
                switch (error_value) {
                    case 2002:  // CR_CONNECTION_ERROR
                    case 2003:  // CR_CONN_HOST_ERROR
                    case 2005:  // CR_UNKNOWN_HOST
                    case 2006:  // CR_SERVER_GONE_ERROR
                    case 2013:  // CR_SERVER_LOST
                        is_connection_error = true;
                        break;
                }
            }

            if (!is_connection_error || !co_await m_impl->connect->tryConnect()) {
                HKU_ERROR(
                  "Failed prepare statement! Error code: {}|{}, Server message: {}, Client "
                  "message: {}",
                  ec.value(), ec.message(), diag.server_message(), diag.client_message());
                SQL_THROW(ec.value(), "Failed prepare statement! {}", m_sql_string);
            }

            // Prepare it again after the reconnection
            m_impl->stmt = co_await m_impl->connect->m_impl->get_statement(m_sql_string, ec, diag);
            if (ec) {
                HKU_ERROR(
                  "Failed prepare statement after reconnect! Error code: {}, Server message: {}, "
                  "Client message: {}",
                  ec.value(), diag.server_message(), diag.client_message());
                SQL_THROW(ec.value(), "Failed prepare statement! {}", m_sql_string);
            }
        }
    } else {
        // Reuse the existing prepared statement and reset the execution state only
        m_impl->results = {};           // Reset results
        m_impl->exec_state = {};        // Reset the execution state
        m_impl->current_batch.clear();  // Clear the current batch
        // Note: do not clear params here, because bind has already been called before exec
        // m_impl->params.clear();
        m_impl->current_row = 0;
        m_impl->total_rows_read = 0;
        m_impl->has_result = false;
        m_impl->is_streaming = false;
    }

    auto* conn = static_cast<boost::mysql::tcp_connection*>(m_impl->connect->getRawConnection());

    try {
        boost::mysql::diagnostics diag;
        if (m_impl->params.empty()) {
            // Without parameters, use the streaming execution
            co_await conn->async_start_execution(m_sql_string, m_impl->exec_state, diag,
                                                 boost::asio::use_awaitable);
        } else {
            // With parameters, use the prepared statement execution
            std::vector<boost::mysql::field_view> param_views;
            param_views.reserve(m_impl->params.size());
            for (const auto& f : m_impl->params) {
                param_views.push_back(boost::mysql::field_view(f));
            }

            // Bind the parameters
            auto bound = m_impl->stmt->bind(param_views.begin(), param_views.end());

            // Use the streaming execution, consistent with the case without parameters
            co_await conn->async_start_execution(bound, m_impl->exec_state, diag,
                                                 boost::asio::use_awaitable);
        }

        m_impl->is_streaming = true;
        m_impl->current_row = 0;
        m_impl->total_rows_read = 0;

        // Read the first batch of data when the result set needs to be read
        if (m_impl->exec_state.should_read_rows()) {
            boost::mysql::diagnostics read_diag;
            boost::mysql::rows_view batch_view = co_await conn->async_read_some_rows(
              m_impl->exec_state, read_diag, boost::asio::use_awaitable);

            // Convert rows_view into vector<row> to own the data
            m_impl->current_batch.assign(batch_view.begin(), batch_view.end());
            m_impl->total_rows_read += m_impl->current_batch.size();
        }
    } catch (const boost::mysql::error_with_diagnostics& e) {
        HKU_ERROR("Execute failed! Error code: {}, Server message: {}, Client message: {}",
                  e.code().value(), e.get_diagnostics().server_message(),
                  e.get_diagnostics().client_message());
        SQL_THROW(e.code().value(), "Failed execute sql: {}! {}", m_sql_string, e.code().message());
    } catch (const boost::system::system_error& e) {
        HKU_ERROR("Execute failed! Error code: {}, Message: {}", e.code().value(),
                  e.code().message());
        SQL_THROW(e.code().value(), "Failed execute sql: {}! {}", m_sql_string, e.code().message());
    }

    m_impl->has_result = true;
    m_impl->needs_reset = true;

    // Clear the parameters so that the index starts from 0 at the next bind
    m_impl->params.clear();
}

net::awaitable<bool> AsyncMySQLStatement::sub_moveNext() {
    if (!m_impl->has_result) {
        _reset();
        co_return false;
    }

    auto* conn = static_cast<boost::mysql::tcp_connection*>(m_impl->connect->getRawConnection());

    if (m_impl->is_streaming) {
        // The streaming mode
        m_impl->current_row++;

        // If the current batch still has data, return directly
        if (m_impl->current_row <= m_impl->current_batch.size()) {
            co_return true;
        }

        // The current batch has been read, try to read the next batch
        if (m_impl->exec_state.should_read_rows()) {
            try {
                boost::mysql::diagnostics diag;
                boost::mysql::rows_view batch_view = co_await conn->async_read_some_rows(
                  m_impl->exec_state, diag, boost::asio::use_awaitable);

                // Convert rows_view into vector<row>
                m_impl->current_batch.assign(batch_view.begin(), batch_view.end());
                m_impl->total_rows_read += m_impl->current_batch.size();
                m_impl->current_row = 1;  // Reset to the first row of the first batch

                co_return !m_impl->current_batch.empty();
            } catch (...) {
                // The reading failed, end the iteration
                _reset();
                co_return false;
            }
        } else {
            // There is no more data
            _reset();
            co_return false;
        }
    } else {
        // The non-streaming mode
        const auto& rows = m_impl->results.rows();
        if (m_impl->current_row >= rows.size()) {
            _reset();
            co_return false;
        }

        m_impl->current_row++;
        co_return true;
    }
}

uint64_t AsyncMySQLStatement::sub_getLastRowid() {
    if (m_impl->is_streaming) {
        // The streaming mode: get it from execution_state
        return m_impl->exec_state.last_insert_id();
    } else {
        // The non-streaming mode: get it from results
        return m_impl->results.last_insert_id();
    }
}

void AsyncMySQLStatement::_reset() {
    if (m_impl->needs_reset) {
        m_impl->results = {};           // Reset results
        m_impl->exec_state = {};        // Reset the execution state
        m_impl->current_batch.clear();  // Clear the current batch
        m_impl->params.clear();
        m_impl->params.shrink_to_fit();
        m_impl->current_row = 0;
        m_impl->total_rows_read = 0;
        m_impl->has_result = false;
        m_impl->is_streaming = false;
    }
}

void AsyncMySQLStatement::sub_bindNull(int idx) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    m_impl->params.push_back(boost::mysql::field());  // Constructed as NULL by default
}

void AsyncMySQLStatement::sub_bindInt(int idx, int64_t value) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    m_impl->params.push_back(boost::mysql::field(static_cast<std::int64_t>(value)));
}

void AsyncMySQLStatement::sub_bindDouble(int idx, double item) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    m_impl->params.push_back(boost::mysql::field(item));
}

void AsyncMySQLStatement::sub_bindDatetime(int idx, const Datetime& item) {
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

void AsyncMySQLStatement::sub_bindText(int idx, const std::string& item) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    m_impl->params.push_back(boost::mysql::field(item));
}

void AsyncMySQLStatement::sub_bindText(int idx, const char* item, size_t len) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    std::string str(item, len);
    m_impl->params.push_back(boost::mysql::field(str));
}

void AsyncMySQLStatement::sub_bindBlob(int idx, const std::string& item) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    std::vector<unsigned char> blob(item.begin(), item.end());
    m_impl->params.push_back(boost::mysql::field(blob));
}

void AsyncMySQLStatement::sub_bindBlob(int idx, const std::vector<char>& item) {
    SQL_CHECK(idx == static_cast<int>(m_impl->params.size()), -1,
              "Parameter index must be sequential! Expected index: {}, but got: {}",
              m_impl->params.size(), idx);
    std::vector<unsigned char> blob(item.begin(), item.end());
    m_impl->params.push_back(boost::mysql::field(blob));
}

int AsyncMySQLStatement::sub_getNumColumns() const {
    HKU_IF_RETURN(!m_impl->has_result, 0);

    if (m_impl->is_streaming) {
        // The streaming mode: get the metadata from the current batch
        if (m_impl->current_batch.empty()) {
            return 0;
        }
        return static_cast<int>(m_impl->current_batch[0].size());
    } else {
        // The non-streaming mode: get it from results
        const auto& metadata = m_impl->results.meta();
        HKU_IF_RETURN(metadata.empty(), 0);
        return static_cast<int>(metadata.size());
    }
}

void AsyncMySQLStatement::sub_getColumnAsInt64(int idx, int64_t& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& value = m_impl->getField(idx);
    if (value.is_null()) {
        item = 0;
        return;
    }

    try {
        item = value.as_int64();
    } catch (...) {
        try {
            uint64_t u = value.as_uint64();
            item = static_cast<int64_t>(u);
        } catch (...) {
            try {
                std::string str = value.as_string();
                item = std::stoll(str);
            } catch (const std::exception& e) {
                SQL_THROW(-1, "Failed to convert column {} to int64: {}", idx, e.what());
            }
        }
    }
}

void AsyncMySQLStatement::sub_getColumnAsDouble(int idx, double& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& value = m_impl->getField(idx);
    if (value.is_null()) {
        item = 0.0;
        return;
    }

    try {
        item = value.as_double();
    } catch (...) {
        try {
            float f = value.as_float();
            item = static_cast<double>(f);
        } catch (...) {
            try {
                std::string str = value.as_string();
                item = std::stod(str);
            } catch (const std::exception& e) {
                SQL_THROW(-1, "Failed to convert column {} to double: {}", idx, e.what());
            }
        }
    }
}

void AsyncMySQLStatement::sub_getColumnAsDatetime(int idx, Datetime& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& value = m_impl->getField(idx);
    if (value.is_null()) {
        item = Null<Datetime>();
        return;
    }

    try {
        auto dt = value.as_datetime();
        item = Datetime(static_cast<long>(dt.year()), static_cast<long>(dt.month()),
                        static_cast<long>(dt.day()), static_cast<long>(dt.hour()),
                        static_cast<long>(dt.minute()), static_cast<long>(dt.second()),
                        static_cast<long>(dt.microsecond() / 1000),
                        static_cast<long>(dt.microsecond() % 1000));
    } catch (...) {
        try {
            auto d = value.as_date();
            item = Datetime(static_cast<long>(d.year()), static_cast<long>(d.month()),
                            static_cast<long>(d.day()));
        } catch (...) {
            std::string datetime_str = value.as_string();
            item = Datetime(datetime_str);
        }
    }
}

void AsyncMySQLStatement::sub_getColumnAsText(int idx, std::string& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& value = m_impl->getField(idx);
    if (value.is_null()) {
        item.clear();
        return;
    }

    try {
        item = value.as_string();
    } catch (...) {
        try {
            auto dt = value.as_datetime();
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d", dt.year(), dt.month(),
                     dt.day(), dt.hour(), dt.minute(), dt.second());
            item = buffer;
        } catch (...) {
            item = value.as_string();
        }
    }
}

void AsyncMySQLStatement::sub_getColumnAsBlob(int idx, std::string& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& value = m_impl->getField(idx);
    if (value.is_null()) {
        item.clear();
        return;
    }

    try {
        auto blob = value.as_blob();
        item.assign(reinterpret_cast<const char*>(blob.data()), blob.size());
    } catch (...) {
        item = value.as_string();
    }
}

void AsyncMySQLStatement::sub_getColumnAsBlob(int idx, std::vector<char>& item) {
    SQL_CHECK(m_impl->has_result, -1, "No result available!");

    const auto& value = m_impl->getField(idx);
    if (value.is_null()) {
        item.clear();
        return;
    }

    try {
        auto blob = value.as_blob();
        item.assign(blob.begin(), blob.end());
    } catch (...) {
        std::string str = value.as_string();
        item.assign(str.begin(), str.end());
    }
}

}  // namespace hku
