/*
 * AsyncMySQLConnect.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */

#include <memory>
#include <boost/mysql.hpp>
#include <boost/asio.hpp>

#include "hikyuu/utilities/config.h"
#include "hikyuu/utilities/LruCache.h"
#include "hikyuu/utilities/Parameter.h"
#include "hikyuu/utilities/Log.h"
#include "AsyncMySQLConnect.h"

namespace hku {

// 辅助函数：打印 diagnostics 诊断信息（内部使用，不对外暴露）
static void printAsyncMySQLDiag(const boost::mysql::error_code& ec,
                                const boost::mysql::diagnostics& diag, const std::string& context) {
    if (!diag.server_message().empty()) {
        HKU_ERROR("{} Server error: {}", context, diag.server_message());
    } else if (!diag.client_message().empty()) {
        HKU_ERROR("{} Client error: {}", context, diag.client_message());
    } else {
        HKU_ERROR("{} Error code {}: {}", context, ec.value(), ec.message());
    }
}

// Pimpl 实现结构体
struct AsyncMySQLConnect::Impl {
    boost::asio::io_context* io_context_ptr = nullptr;  // 指向外部 io_context，不拥有
    std::unique_ptr<boost::mysql::tcp_connection> conn;
    std::unique_ptr<LruCache<std::string, std::shared_ptr<boost::mysql::statement>>>
      statement_cache;
    bool initialized = false;

    Impl() {}

    ~Impl() {
        statement_cache.reset();
        if (conn) {
            conn->close();
        }
    }

    // 从当前协程环境获取 io_context（只在首次调用时执行）
    net::awaitable<void> ensure_initialized() {
        if (!initialized) {
            auto executor = co_await net::this_coro::executor;
            io_context_ptr = &static_cast<boost::asio::io_context&>(executor.context());
            initialized = true;
        }
        co_return;
    }

    net::awaitable<std::shared_ptr<boost::mysql::statement>> get_statement(
      const std::string& sql, boost::mysql::error_code& ec, boost::mysql::diagnostics& diag) {
        std::shared_ptr<boost::mysql::statement> ret;
        if (statement_cache->tryGet(sql, ret)) {
            co_return ret;
        }

        // 创建 statement 并准备关闭的 lambda
        auto* connection_ptr = conn.get();
        auto deleter = [connection_ptr](boost::mysql::statement* stmt) {
            if (stmt && connection_ptr) {
                connection_ptr->close_statement(*stmt);
            }
            delete stmt;
        };

        // 使用 async_prepare_statement
        boost::mysql::diagnostics prep_diag;
        try {
            boost::mysql::statement stmt =
              co_await conn->async_prepare_statement(sql, prep_diag, boost::asio::use_awaitable);

            ret = std::shared_ptr<boost::mysql::statement>(
              new boost::mysql::statement(std::move(stmt)), deleter);
            statement_cache->insert(sql, ret);
            co_return ret;
        } catch (const boost::mysql::error_with_diagnostics& e) {
            ec = e.code();
            diag = e.get_diagnostics();
            co_return nullptr;
        } catch (const boost::system::system_error& e) {
            ec = e.code();
            co_return nullptr;
        }
    }
};

AsyncMySQLConnect::AsyncMySQLConnect(const Parameter& param)
: AsyncDBConnectBase(param), m_impl(std::make_unique<Impl>()) {
    // 获取预处理语句缓存大小，并创建缓存
    int64_t cache_size = tryGetParam<int64_t>("statement_cache_size", 3);
    m_params.set("statement_cache_size", cache_size);
    m_impl->statement_cache =
      std::make_unique<LruCache<std::string, std::shared_ptr<boost::mysql::statement>>>(cache_size);
}

AsyncMySQLConnect::~AsyncMySQLConnect() {
    close();
}

void* AsyncMySQLConnect::getRawConnection() const {
    HKU_ASSERT(m_impl->initialized);
    return m_impl->conn.get();
}

net::awaitable<bool> AsyncMySQLConnect::tryConnect() {
    try {
        close();
        co_await connect();
        co_return true;
    } catch (const std::exception& e) {
        HKU_WARN("{}", e.what());
        co_return false;
    }
}

net::awaitable<void> AsyncMySQLConnect::connect() {
    // 确保已初始化（从协程环境获取 io_context）
    co_await m_impl->ensure_initialized();

    std::string host = tryGetParam<std::string>("host", "127.0.0.1");
    std::string usr = tryGetParam<std::string>("usr", "root");
    std::string pwd = tryGetParam<std::string>("pwd", "");
    std::string database = tryGetParam<std::string>("db", "");
    unsigned short port = static_cast<unsigned short>(tryGetParam<int>("port", 3306));

    m_impl->conn = std::make_unique<boost::mysql::tcp_connection>(*m_impl->io_context_ptr);
    boost::mysql::handshake_params params(usr, pwd, database);

    boost::mysql::diagnostics diag;

    try {
        co_await m_impl->conn->async_connect(
          boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host), port), params, diag,
          boost::asio::use_awaitable);
    } catch (const boost::mysql::error_with_diagnostics& e) {
        printAsyncMySQLDiag(e.code(), e.get_diagnostics(), "MySQL connect");
        throw exception(fmt::format("EXCEPTION: {} [{}] ({}:{})",
                                    fmt::format("{}, {}", e.code().value(), e.code().message()), "",
                                    __FILE__, __LINE__));
    } catch (const boost::system::system_error& e) {
        throw exception(
          fmt::format("EXCEPTION: {} [{}] ({}:{})",
                      fmt::format("MySQL connect failed: {}, {}", e.code().value(), e.what()), "",
                      __FILE__, __LINE__));
    }
}

void AsyncMySQLConnect::close() {
    if (m_impl && m_impl->conn) {
        if (m_impl->statement_cache) {
            m_impl->statement_cache->clear();
        }
        m_impl->conn->close();
        m_impl->conn.reset();
        m_impl->io_context_ptr = nullptr;
        m_impl->initialized = false;
    }
}

net::awaitable<bool> AsyncMySQLConnect::ping() {
    if (!m_impl || !m_impl->conn) {
        bool connected = co_await tryConnect();
        if (!connected) {
            HKU_ERROR("Failed connect to mysql!");
            co_return false;
        }
    }

    boost::mysql::results results;
    boost::mysql::diagnostics diag;
    bool need_reconnect = false;

    try {
        co_await m_impl->conn->async_execute("SELECT 1", results, diag, boost::asio::use_awaitable);
        co_return true;
    } catch (const boost::mysql::error_with_diagnostics&) {
        need_reconnect = true;
    } catch (const boost::system::system_error&) {
        need_reconnect = true;
    }

    // 在 try-catch 外部处理重连
    if (need_reconnect) {
        bool reconnected = false;
        try {
            reconnected = co_await tryConnect();
        } catch (...) {
            // 忽略重连异常
        }

        if (!reconnected) {
            printAsyncMySQLDiag(boost::mysql::error_code(), diag, "MySQL ping failed!");
            co_return false;
        }
        co_return true;
    }

    co_return true;
}

net::awaitable<int64_t> AsyncMySQLConnect::exec(const std::string& sql_string) {
#if HKU_SQL_TRACE
    HKU_DEBUG(sql_string);
#endif

    if (!m_impl || !m_impl->conn) {
        SQL_CHECK(co_await tryConnect(), -1, "Failed connect to mysql!");
    }

    boost::mysql::results results;
    boost::mysql::diagnostics diag;
    boost::mysql::error_code ec;
    boost::mysql::diagnostics err_diag;
    bool need_retry = false;

    try {
        co_await m_impl->conn->async_execute(sql_string, results, diag, boost::asio::use_awaitable);
    } catch (const boost::mysql::error_with_diagnostics& e) {
        ec = e.code();
        err_diag = e.get_diagnostics();
        need_retry = true;
    } catch (const boost::system::system_error& e) {
        ec = e.code();
        printAsyncMySQLDiag(ec, diag, "MySQL execute sql");
        SQL_THROW(ec.value(), "SQL error: {}! error msg: {}", sql_string, ec.message());
    }

    // 如果需要重试，在 try-catch 外部执行
    if (need_retry) {
        bool reconnected = false;
        try {
            reconnected = co_await ping();
        } catch (...) {
            // 忽略 ping 异常
        }

        if (reconnected) {
            try {
                co_await m_impl->conn->async_execute(sql_string, results, diag,
                                                     boost::asio::use_awaitable);
            } catch (const boost::mysql::error_with_diagnostics& retry_e) {
                ec = retry_e.code();
                err_diag = retry_e.get_diagnostics();
            } catch (const boost::system::system_error& retry_e) {
                ec = retry_e.code();
            }
        }

        if (ec) {
            printAsyncMySQLDiag(ec, err_diag, "MySQL execute sql");
            SQL_THROW(ec.value(), "SQL error: {}! error msg: {}", sql_string, ec.message());
        }
    }

    co_return results.affected_rows();
}

net::awaitable<AsyncSQLStatementPtr> AsyncMySQLConnect::getStatement(
  const std::string& sql_statement) {
    co_return std::make_shared<AsyncMySQLStatement>(this, sql_statement);
}

net::awaitable<bool> AsyncMySQLConnect::tableExist(const std::string& tablename) {
    bool result = false;
    try {
        auto st = co_await getStatement(fmt::format("SELECT 1 FROM {} LIMIT 1;", tablename));
        co_await st->exec();
        result = true;
    } catch (...) {
        result = false;
    }
    co_return result;
}

net::awaitable<void> AsyncMySQLConnect::resetAutoIncrement(const std::string& tablename) {
    int64_t count =
      co_await queryNumber<int64_t>(fmt::format("select count(1) from {}", tablename));
    SQL_CHECK(count == 0, -1, "The ID cannot be reset when data is present in table({})",
              tablename);
    co_await exec(fmt::format("alter {} auto_increment=1", tablename));
}

net::awaitable<void> AsyncMySQLConnect::transaction() {
    co_await exec("BEGIN");
}

net::awaitable<void> AsyncMySQLConnect::commit() {
    co_await exec("COMMIT");
}

net::awaitable<void> AsyncMySQLConnect::rollback() noexcept {
    try {
        co_await exec("ROLLBACK");
    } catch (const std::exception& e) {
        HKU_ERROR("Failed transaction! {}", e.what());
    } catch (...) {
        HKU_ERROR("Unknown error!");
    }
}

}  // namespace hku