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
    // boost.mysql 的 statement 会自动清理
}

void MySQLStatement::_prepare() {
    try {
        boost::mysql::error_code ec;
        boost::mysql::diagnostics diag;
        _reset();

        // 使用 MySQLConnect 的 impl 里的 get_statement 方法
        m_impl->stmt = m_impl->connect->m_impl->get_statement(m_sql_string, ec, diag);
        m_impl->needs_reset = true;

        if (ec) [[unlikely]] {
            _reset();

            // 判断是否为连接层错误（需要重连），而非 SQL 应用层错误
            // MySQL 客户端连接错误码:
            // - 2013 (CR_SERVER_LOST): 查询过程中丢失连接
            // - 2006 (CR_SERVER_GONE_ERROR): 服务器已断开
            // - 2003 (CR_CONN_HOST_ERROR): 无法连接到服务器
            // - 2002 (CR_CONNECTION_ERROR): 本地连接失败
            // - 2005 (CR_UNKNOWN_HOST): 未知主机
            bool is_connection_error = false;

            // 通过错误码判断（Boost.MySQL 使用 boost::system::error_code）
            int error_value = ec.value();

            // MySQL 客户端错误范围是 2000-2999
            if (error_value >= 2000 && error_value <= 2999) {
                // 常见的连接相关错误码
                switch (error_value) {
                    case 2002:  // CR_CONNECTION_ERROR
                    case 2003:  // CR_CONN_HOST_ERROR
                    case 2005:  // CR_UNKNOWN_HOST
                    case 2006:  // CR_SERVER_GONE_ERROR
                    case 2013:  // CR_SERVER_LOST
                        is_connection_error = true;
                        break;
                    default:
                        // 其他客户端错误，检查消息中是否包含连接相关关键词
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

            // 只在连接层错误时尝试重连
            if (is_connection_error) {
                _reset();
                if (m_impl->connect->ping()) {
                    // ping 成功（已自动重连），再次获取 statement
                    m_impl->stmt = m_impl->connect->m_impl->get_statement(m_sql_string, ec, diag);
                    m_impl->needs_reset = true;

                    if (ec) [[unlikely]] {
                        // 重连后仍然失败，打印错误日志
                        HKU_ERROR("Failed prepare statement after reconnect! Error code {}: {}",
                                  ec.value(), ec.message());
                        SQL_THROW(ec.value(), "Failed prepare statement after reconnect!");
                    }
                    return;
                }
            }

            // 非连接错误或重连失败，直接抛出原始错误
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

    // 获取底层连接用于执行
    auto* conn = static_cast<boost::mysql::tcp_connection*>(m_impl->connect->getRawConnection());

    if (m_impl->params.empty()) {
        // 没有参数，直接执行
        conn->execute(m_sql_string, m_impl->results, ec, diag);
    } else {
        // 有参数，使用预处理语句（统一使用 field_view 迭代器）
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
    m_impl->params.push_back(boost::mysql::field());  // 默认构造为 NULL
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

    // 使用 boost.mysql 原生 datetime 类型
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
        // 尝试直接转换为 int64
        item = value.as_int64();
    } catch (...) {
        try {
            // 尝试作为 uint64 转换（YEAR 可能以 uint64 返回）
            uint64_t u = value.as_uint64();
            item = static_cast<int64_t>(u);
        } catch (...) {
            try {
                // 最后尝试作为字符串解析
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
        // 尝试直接转换为 double
        item = value.as_double();
    } catch (...) {
        try {
            // 如果失败，尝试作为 float 转换
            float f = value.as_float();
            item = static_cast<double>(f);
        } catch (...) {
            try {
                // 最后尝试作为字符串解析（DECIMAL 类型可能以字符串返回）
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
        // 优先尝试直接作为 datetime 读取
        auto dt = value.as_datetime();
        item = Datetime(static_cast<long>(dt.year()), static_cast<long>(dt.month()),
                        static_cast<long>(dt.day()), static_cast<long>(dt.hour()),
                        static_cast<long>(dt.minute()), static_cast<long>(dt.second()),
                        static_cast<long>(dt.microsecond() / 1000),   // 微秒转毫秒
                        static_cast<long>(dt.microsecond() % 1000));  // 剩余微秒
    } catch (...) {
        try {
            // 尝试作为 date 读取（没有时间部分）
            auto d = value.as_date();
            item = Datetime(static_cast<long>(d.year()), static_cast<long>(d.month()),
                            static_cast<long>(d.day()));
        } catch (...) {
            // 最后尝试作为字符串解析
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
        // 尝试直接转换为字符串
        item = value.as_string();
    } catch (...) {
        // 如果失败，尝试其他类型转换
        try {
            // 尝试 date 类型
            auto d = value.as_date();
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", d.year(),
                     static_cast<int>(d.month()), static_cast<int>(d.day()));
            item = std::string(buffer);
        } catch (...) {
            try {
                // 尝试 datetime 类型
                auto dt = value.as_datetime();
                char buffer[30];
                snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d", dt.year(),
                         static_cast<int>(dt.month()), static_cast<int>(dt.day()),
                         static_cast<int>(dt.hour()), static_cast<int>(dt.minute()),
                         static_cast<int>(dt.second()));
                item = std::string(buffer);
            } catch (...) {
                try {
                    // 尝试 time 类型（boost.mysql 的 time 是 std::chrono::microseconds）
                    auto t = value.as_time();
                    auto total_seconds =
                      std::chrono::duration_cast<std::chrono::seconds>(t).count();
                    int hours = total_seconds / 3600;
                    int minutes = (total_seconds % 3600) / 60;
                    int seconds = total_seconds % 60;
                    char buffer[20];
                    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
                    item = std::string(buffer);
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
        const auto& blob = value.as_blob();
        item.assign(blob.begin(), blob.end());
    } catch (...) {
        SQL_THROW(-1, "Failed to convert column {} to blob", idx);
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
        SQL_THROW(-1, "Failed to convert column {} to blob", idx);
    }
}

uint64_t MySQLStatement::sub_getLastRowid() {
    return m_impl->results.last_insert_id();
}

}  // namespace hku