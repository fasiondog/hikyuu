/*
 * AsyncSQLiteConnect.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */

#include "hikyuu/utilities/config.h"
#include "hikyuu/utilities/Log.h"
#include "AsyncSQLiteConnect.h"
#include <sqlite3.h>
#include <thread>
#include "hikyuu/utilities/thread/algorithm.h"

namespace hku {

// sqlite3 多线程处理时，等待其他锁释放回调处理
static int sqlite_busy_call_back_in_async(void *ptr, int count) {
    std::this_thread::yield();
    return 1;
}

// Pimpl 实现结构体
struct AsyncSQLiteConnect::Impl {
    sqlite3 *m_db = nullptr;
    std::string m_dbname;
    bool initialized = false;
    ThreadPool m_thread_pool{1};  // 单线程池用于执行同步 SQLite 操作
};

AsyncSQLiteConnect::AsyncSQLiteConnect(const Parameter &param)
: AsyncDBConnectBase(param), m_impl(std::make_unique<Impl>()) {
    // 注意：构造函数中不能使用 co_await，连接在首次使用时建立
    try {
        m_impl->m_dbname = getParam<std::string>("db");
    } catch (std::out_of_range &e) {
        HKU_FATAL("Can't get database name! {}", e.what());
        throw;
    }
}

AsyncSQLiteConnect::~AsyncSQLiteConnect() {
    close();
}

void *AsyncSQLiteConnect::getRawConnection() const noexcept {
    return m_impl->m_db;
}

ThreadPool::ExecutorWrapper AsyncSQLiteConnect::getThreadPoolExecutor() const noexcept {
    return m_impl->m_thread_pool.executor();
}

net::awaitable<void> AsyncSQLiteConnect::connect() {
    if (m_impl->initialized) {
        co_return;
    }

    // 在线程池中执行同步初始化操作
    auto init_func = [this]() -> int {
        try {
            _connect();
            return SQLITE_OK;
        } catch (const SQLException &e) {
            return e.errcode();
        }
    };

    int rc = co_await co_run(m_impl->m_thread_pool.executor(), init_func);

    SQL_CHECK(rc == SQLITE_OK, rc, "{}",
              m_impl->m_db ? sqlite3_errmsg(m_impl->m_db) : "Failed to open database");
}

void AsyncSQLiteConnect::_connect() {
    if (m_impl->initialized) {
        return;
    }

    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
    if (haveParam("flags")) {
        flags = getParam<int>("flags");
    }

#if HKU_ENABLE_SQLCIPHER
    std::string key;
    if (haveParam("key")) {
        key = getParam<std::string>("key");
    }
#endif

    // 1. 打开数据库
    int rc = sqlite3_open_v2(m_impl->m_dbname.c_str(), &m_impl->m_db, flags, NULL);
    SQL_CHECK(rc == SQLITE_OK, rc, "{}",
              m_impl->m_db ? sqlite3_errmsg(m_impl->m_db) : "Failed to open database");

#if HKU_ENABLE_SQLCIPHER
    // 2. 设置密钥（如果需要）
    if (!key.empty()) {
        rc = sqlite3_key(m_impl->m_db, key.c_str(), static_cast<int>(key.size()));
        SQL_CHECK(rc == SQLITE_OK, rc, "{}", sqlite3_errmsg(m_impl->m_db));
    }
#endif

    // 3. 设置 busy handler
    sqlite3_busy_handler(m_impl->m_db, sqlite_busy_call_back_in_async, (void *)m_impl->m_db);

    // 4. 启用扩展错误码
    if (sqlite3_libversion_number() >= 3003008) {
        sqlite3_extended_result_codes(m_impl->m_db, true);
    }

    m_impl->initialized = true;
}

void AsyncSQLiteConnect::close() {
    if (m_impl && m_impl->m_db) {
        sqlite3_close(m_impl->m_db);
        m_impl->m_db = nullptr;
        m_impl->initialized = false;
    }
}

net::awaitable<bool> AsyncSQLiteConnect::ping() {
    if (!m_impl || !m_impl->m_db) {
        try {
            co_await connect();
        } catch (const std::exception &e) {
            HKU_ERROR("Failed connect to sqlite! {}", e.what());
            co_return false;
        }
    }

    // sqlite打开时并不会对文件是否是有效sqlite文件进行检查，
    // 只有执行 sql 语句时，才会报 SQLITE_NOTADB(26) 错误
    auto ping_func = [this]() -> int {
        return sqlite3_exec(m_impl->m_db, "PRAGMA synchronous;", NULL, NULL, NULL);
    };

    int rc = co_await co_run(m_impl->m_thread_pool.executor(), ping_func);
    co_return (rc == SQLITE_OK);
}

net::awaitable<int64_t> AsyncSQLiteConnect::exec(const std::string &sql_string) {
#if HKU_SQL_TRACE
    HKU_DEBUG(sql_string);
#endif

    if (!m_impl || !m_impl->m_db) {
        co_await connect();
    }

    auto exec_func = [this, &sql_string]() -> std::pair<int, int> {
        int rc = sqlite3_exec(m_impl->m_db, sql_string.c_str(), NULL, NULL, NULL);
        int affect_rows = sqlite3_changes(m_impl->m_db);
        return {rc, affect_rows};
    };

    auto [rc, affect_rows] = co_await co_run(m_impl->m_thread_pool.executor(), exec_func);

    SQL_CHECK(rc == SQLITE_OK, rc, "SQL error: {}! ({})",
              m_impl->m_db ? sqlite3_errmsg(m_impl->m_db) : "Unknown error", sql_string);

    co_return (affect_rows < 0 ? 0 : affect_rows);
}

net::awaitable<AsyncSQLStatementPtr> AsyncSQLiteConnect::getStatement(
  const std::string &sql_statement) {
    if (!m_impl || !m_impl->m_db) {
        co_await connect();
    }

    co_return std::make_shared<AsyncSQLiteStatement>(this, sql_statement);
}

net::awaitable<bool> AsyncSQLiteConnect::tableExist(const std::string &tablename) {
    bool result = false;
    try {
        auto st = co_await getStatement(
          fmt::format("select count(1) from sqlite_master where name='{}'", tablename));
        co_await st->exec();
        if (co_await st->moveNext()) {
            int tmp;
            st->getColumn(0, tmp);
            if (tmp == 1) {
                result = true;
            }
        }
    } catch (...) {
        result = false;
    }
    co_return result;
}

net::awaitable<void> AsyncSQLiteConnect::resetAutoIncrement(const std::string &tablename) {
    int64_t count =
      co_await queryNumber<int64_t>(fmt::format("select count(1) from {}", tablename));
    SQL_CHECK(count == 0, -1, "The ID cannot be reset when data is present in table({})",
              tablename);
    co_await exec(fmt::format("UPDATE sqlite_sequence SET seq=0 WHERE name='{}'", tablename));
}

net::awaitable<void> AsyncSQLiteConnect::transaction() {
    co_await exec("BEGIN IMMEDIATE");
}

net::awaitable<void> AsyncSQLiteConnect::commit() {
    co_await exec("COMMIT TRANSACTION");
}

net::awaitable<void> AsyncSQLiteConnect::rollback() noexcept {
    try {
        co_await exec("ROLLBACK TRANSACTION");
    } catch (const std::exception &e) {
        HKU_ERROR("Failed rollback! {}", e.what());
    } catch (...) {
        HKU_ERROR("Unknown error!");
    }
}

net::awaitable<bool> AsyncSQLiteConnect::check(bool quick) {
    if (!m_impl || !m_impl->m_db) {
        co_await connect();
    }

    std::string check_pragma(quick ? "PRAGMA quick_check;" : "PRAGMA integrity_check;");

    auto check_func = [this, &check_pragma]() -> bool {
        bool good = false;
        sqlite3_stmt *integrity = NULL;

        if (sqlite3_prepare_v2(m_impl->m_db, check_pragma.c_str(), -1, &integrity, NULL) ==
            SQLITE_OK) {
            while (sqlite3_step(integrity) == SQLITE_ROW) {
                const unsigned char *result = sqlite3_column_text(integrity, 0);
                if (result && strcmp((const char *)result, (const char *)"ok") == 0) {
                    good = true;
                    break;
                }
            }
            sqlite3_finalize(integrity);
        }

        return good;
    };

    bool result = co_await co_run(m_impl->m_thread_pool.executor(), check_func);
    co_return result;
}

net::awaitable<bool> AsyncSQLiteConnect::backup(const char *zFilename, int n_page, int step_sleep) {
    if (!m_impl || !m_impl->m_db) {
        co_await connect();
    }

    auto backup_func = [this, zFilename, n_page, step_sleep]() -> bool {
        sqlite3 *pFile;
        int rc = sqlite3_open(zFilename, &pFile);
        if (rc == SQLITE_OK) {
            /* Open the sqlite3_backup object used to accomplish the transfer */
            sqlite3_backup *pBackup = sqlite3_backup_init(pFile, "main", m_impl->m_db, "main");
            if (pBackup) {
                if (n_page <= 0) {
                    sqlite3_backup_step(pBackup, -1);

                } else {
                    do {
                        rc = sqlite3_backup_step(pBackup, n_page);
                        if (step_sleep > 0 &&
                            (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED)) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(step_sleep));
                        }
                    } while (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
                }

                sqlite3_backup_finish(pBackup);
            }
            rc = sqlite3_errcode(pFile);
        }

        sqlite3_close(pFile);
        return rc == SQLITE_OK;
    };

    bool result = co_await co_run(m_impl->m_thread_pool.executor(), backup_func);
    co_return result;
}

}  // namespace hku
