/*
 * MySQLConnect_boost.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"
#include "MySQLConnect.h"

#include <atomic>
#include <memory>
#include <boost/mysql.hpp>
#include <boost/asio.hpp>
#include "hikyuu/utilities/LruCache.h"

namespace hku {

// Helper function: print the diagnostics information
static void printDiagHelper(const boost::mysql::error_code& ec,
                            const boost::mysql::diagnostics& diag, const std::string& context) {
    if (!diag.server_message().empty()) {
        HKU_ERROR("{} Server error: {}", context, diag.server_message());
    } else if (!diag.client_message().empty()) {
        HKU_ERROR("{} Client error: {}", context, diag.client_message());
    } else {
        HKU_ERROR("{} Error code {}: {}", context, ec.value(), ec.message());
    }
}

// The Pimpl implementation struct
struct MySQLConnect::Impl {
    // Per-connection-generation state shared with the statement deleters, so a statement is not
    // closed on a connection that has been destroyed by a reconnect
    struct StatementCloseState {
        boost::mysql::tcp_connection* conn{nullptr};
        std::atomic_bool alive{true};
    };

    boost::asio::io_context io_context;
    std::unique_ptr<boost::mysql::tcp_connection> conn;
    std::unique_ptr<LruCache<std::string, std::shared_ptr<boost::mysql::statement>>>
      statement_cache;
    // Recreated for every new connection; the deleters hold the state of their own generation
    std::shared_ptr<StatementCloseState> close_state;

    std::shared_ptr<boost::mysql::statement> get_statement(const std::string& sql,
                                                           boost::mysql::error_code& ec,
                                                           boost::mysql::diagnostics& diag) {
        std::shared_ptr<boost::mysql::statement> ret;
        if (statement_cache->tryGet(sql, ret)) {
            return ret;
        }

        // The deleter closes the statement only while its connection generation is alive
        auto state = close_state;
        auto deleter = [state](boost::mysql::statement* stmt) {
            if (stmt && state->alive.load(std::memory_order_acquire)) {
                boost::mysql::error_code close_ec;
                boost::mysql::diagnostics close_diag;
                state->conn->close_statement(*stmt, close_ec, close_diag);
                // Ignore the closing error, as the connection may already have been lost
            }
            delete stmt;
        };

        ret = std::shared_ptr<boost::mysql::statement>(
          new boost::mysql::statement(conn->prepare_statement(sql, ec, diag)), deleter);

        if (!ec) {
            statement_cache->insert(sql, ret);
        } else {
            ret.reset();
        }

        return ret;
    }
};

MySQLConnect::MySQLConnect(const Parameter& param)
: DBConnectBase(param), m_impl(std::make_unique<Impl>()) {
    // Get the prepared statement cache size and create the cache
    int64_t cache_size = tryGetParam<int64_t>("statement_cache_size", 3);
    m_params.set("statement_cache_size", cache_size);
    m_impl->statement_cache =
      std::make_unique<LruCache<std::string, std::shared_ptr<boost::mysql::statement>>>(cache_size);
    connect();
}

MySQLConnect::~MySQLConnect() {
    close();
}

void* MySQLConnect::getRawConnection() const noexcept {
    return m_impl->conn.get();
}

bool MySQLConnect::tryConnect() noexcept {
    bool success = false;
    try {
        close();
        connect();
        success = true;
    } catch (const std::exception& e) {
        HKU_WARN(e.what());
    }
    return success;
}

void MySQLConnect::connect() {
    try {
        std::string host = tryGetParam<std::string>("host", "127.0.0.1");
        std::string usr = tryGetParam<std::string>("usr", "root");
        std::string pwd = tryGetParam<std::string>("pwd", "");
        std::string database = tryGetParam<std::string>("db", "");
        unsigned short port = static_cast<unsigned short>(tryGetParam<int>("port", 3306));

        m_impl->conn = std::make_unique<boost::mysql::tcp_connection>(m_impl->io_context);

        // A new connection generation starts
        m_impl->close_state = std::make_shared<Impl::StatementCloseState>();
        m_impl->close_state->conn = m_impl->conn.get();

        boost::mysql::handshake_params params(usr, pwd, database);

        boost::mysql::error_code ec;
        boost::mysql::diagnostics diag;
        m_impl->conn->connect(
          boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(host), port), params, ec,
          diag);

        if (ec) {
            printDiagHelper(ec, diag, "MySQL connect");
            HKU_THROW("{}, {}", ec.value(), ec.message());
        }

    } catch (const hku::exception& e) {
        close();
        HKU_ERROR(e.what());
        HKU_THROW("Failed create MySQLConnect! {}", e.what());

    } catch (const std::exception& e) {
        close();
        HKU_ERROR(e.what());
        HKU_THROW("Failed create MySQLConnent instance! {}", e.what());

    } catch (...) {
        close();
        const char* errmsg = "Failed create MySQLConnect instance! Unknown error";
        HKU_ERROR(errmsg);
        HKU_THROW("{}", errmsg);
    }
}

void MySQLConnect::close() {
    if (m_impl && m_impl->conn) {
        // Close the cached statements while the connection is still alive, then mark this
        // generation dead before destroying the connection
        m_impl->statement_cache->clear();
        m_impl->close_state->alive.store(false, std::memory_order_release);

        m_impl->conn->close();
        m_impl->conn.reset();
    }
}

bool MySQLConnect::ping() {
    HKU_ERROR_IF_RETURN((!m_impl || !m_impl->conn) && !tryConnect(), false,
                        "Failed connect to mysql!");

    try {
        boost::mysql::error_code ec;
        boost::mysql::diagnostics diag;
        boost::mysql::results results;
        m_impl->conn->execute("SELECT 1", results, ec, diag);

        // Try to reconnect when the ping fails
        if (ec && !tryConnect()) [[unlikely]] {
            printDiagHelper(ec, diag, "MySQL ping failed!");
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        // Try to reconnect on an exception as well
        HKU_ERROR_IF_RETURN(!tryConnect(), false, "MySQL ping exception! {}", e.what());
        return true;
    }
}

int64_t MySQLConnect::exec(const std::string& sql_string) {
#if HKU_SQL_TRACE
    HKU_DEBUG(sql_string);
#endif

    if (!m_impl || !m_impl->conn) {
        SQL_CHECK(tryConnect(), -1, "Failed connect to mysql!");
    }

    boost::mysql::error_code ec;
    boost::mysql::diagnostics diag;
    boost::mysql::results results;
    m_impl->conn->execute(sql_string, results, ec, diag);

    if (ec) [[unlikely]] {
        // Only read-only statements are replayed after a lost connection: a failed write may
        // already have been committed server-side and replaying it would apply it twice
        if (detail::isConnectionLostError(ec.value()) && detail::isReadOnlySql(sql_string) &&
            ping()) {
            m_impl->conn->execute(sql_string, results, ec, diag);
        }

        if (ec) {
            printDiagHelper(ec, diag, "MySQL execute sql");
            SQL_THROW(ec.value(), "SQL error: {}! error msg: {}", sql_string, ec.message());
        }
    }

    // Get the number of the affected rows
    return results.affected_rows();
}

SQLStatementPtr MySQLConnect::getStatement(const std::string& sql_statement) {
    return std::make_shared<MySQLStatement>(this, sql_statement);
}

bool MySQLConnect::tableExist(const std::string& tablename) {
    bool result = false;
    try {
        SQLStatementPtr st =
          getStatement(fmt::format("SELECT 1 FROM {} LIMIT 1;", sqlIdentifier(tablename)));
        st->exec();
        result = true;
    } catch (...) {
        result = false;
    }
    return result;
}

void MySQLConnect::resetAutoIncrement(const std::string& tablename) {
    int64_t count =
      queryNumber<int64_t>(fmt::format("select count(1) from {}", sqlIdentifier(tablename)));
    SQL_CHECK(count == 0, -1, "The ID cannot be reset when data is present in table({})",
              tablename);
    exec(fmt::format("ALTER TABLE {} AUTO_INCREMENT = 1", sqlIdentifier(tablename)));
}

void MySQLConnect::transaction() {
    exec("BEGIN");
}

void MySQLConnect::commit() {
    exec("COMMIT");
}

void MySQLConnect::rollback() noexcept {
    try {
        exec("ROLLBACK");
    } catch (const std::exception& e) {
        HKU_ERROR("Failed transaction! {}", e.what());
    } catch (...) {
        HKU_ERROR("Unknown error!");
    }
}

}  // namespace hku
