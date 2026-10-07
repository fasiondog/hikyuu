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
#include <tuple>
#include "hikyuu/utilities/thread/algorithm.h"

namespace hku {

// The callback handling of waiting for the other locks to be released in the sqlite3 multi-threaded
// processing
static int sqlite_busy_call_back_in_async(void *ptr, int count) {
    std::this_thread::yield();
    return 1;
}

// The Pimpl implementation struct
struct AsyncSQLiteConnect::Impl {
    sqlite3 *m_db = nullptr;
    std::string m_dbname;
    bool initialized = false;
    // Serializes every sqlite3 C API call on m_db: the handle is opened with NOMUTEX and the
    // statement layer touches it from the user threads (prepare/bind/getColumn/finalize) while
    // the step operations run on the pool thread
    mutable std::mutex m_db_mutex;
    std::unique_ptr<ThreadPool> m_thread_pool =
      std::make_unique<ThreadPool>(1);  // A single thread pool used to run the synchronous SQLite
                                        // operations

    ~Impl() {
        // Destroy the pool first: its destructor waits for every queued task to finish, so no
        // sqlite3 call can be in flight while the database handle is closed
        m_thread_pool.reset();
        if (m_db) {
            sqlite3_close_v2(m_db);
            m_db = nullptr;
        }
    }
};

AsyncSQLiteConnect::AsyncSQLiteConnect(const Parameter &param)
: AsyncDBConnectBase(param), m_impl(std::make_unique<Impl>()) {
    // Note: co_await cannot be used in the constructor, the connection is established at the first
    // use
    try {
        m_impl->m_dbname = getParam<std::string>("db");
    } catch (std::out_of_range &e) {
        HKU_FATAL("Can't get database name! {}", e.what());
        throw;
    }
}

AsyncSQLiteConnect::~AsyncSQLiteConnect() = default;

void *AsyncSQLiteConnect::getRawConnection() const noexcept {
    return m_impl->m_db;
}

ThreadPool::ExecutorWrapper AsyncSQLiteConnect::getThreadPoolExecutor() const noexcept {
    return m_impl->m_thread_pool->executor();
}

std::mutex &AsyncSQLiteConnect::getDBMutex() const noexcept {
    return m_impl->m_db_mutex;
}

net::awaitable<void> AsyncSQLiteConnect::connect() {
    if (m_impl->initialized) {
        co_return;
    }

    // Run the synchronous initialization operations in the thread pool
    auto init_func = [this]() -> int {
        try {
            _connect();
            return SQLITE_OK;
        } catch (const SQLException &e) {
            return e.errcode();
        }
    };

    int rc = co_await co_run(m_impl->m_thread_pool->executor(), init_func);

    SQL_CHECK(rc == SQLITE_OK, rc, "{}",
              m_impl->m_db ? sqlite3_errmsg(m_impl->m_db) : "Failed to open database");
}

void AsyncSQLiteConnect::_connect() {
    if (m_impl->initialized) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_impl->m_db_mutex);
    if (m_impl->initialized) {
        return;
    }

    // A previously failed open may have left a half-initialized handle behind, close it before
    // reopening so the retry does not leak it
    if (m_impl->m_db) {
        sqlite3_close_v2(m_impl->m_db);
        m_impl->m_db = nullptr;
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

    // 1. Open the database
    int rc = sqlite3_open_v2(m_impl->m_dbname.c_str(), &m_impl->m_db, flags, NULL);
    SQL_CHECK(rc == SQLITE_OK, rc, "{}",
              m_impl->m_db ? sqlite3_errmsg(m_impl->m_db) : "Failed to open database");

#if HKU_ENABLE_SQLCIPHER
    // 2. Set the key (if needed)
    if (!key.empty()) {
        rc = sqlite3_key(m_impl->m_db, key.c_str(), static_cast<int>(key.size()));
        SQL_CHECK(rc == SQLITE_OK, rc, "{}", sqlite3_errmsg(m_impl->m_db));
    }
#endif

    // 3. Set the busy handler
    sqlite3_busy_handler(m_impl->m_db, sqlite_busy_call_back_in_async, (void *)m_impl->m_db);

    // 4. Enable the extended error codes
    if (sqlite3_libversion_number() >= 3003008) {
        sqlite3_extended_result_codes(m_impl->m_db, true);
    }

    m_impl->initialized = true;
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

    // When sqlite opens a file it does not check whether the file is a valid sqlite file,
    // the SQLITE_NOTADB(26) error is reported only when an sql statement is executed
    auto ping_func = [this]() -> int {
        std::lock_guard<std::mutex> lock(m_impl->m_db_mutex);
        return sqlite3_exec(m_impl->m_db, "PRAGMA synchronous;", NULL, NULL, NULL);
    };

    int rc = co_await co_run(m_impl->m_thread_pool->executor(), ping_func);
    co_return (rc == SQLITE_OK);
}

net::awaitable<int64_t> AsyncSQLiteConnect::exec(const std::string &sql_string) {
#if HKU_SQL_TRACE
    HKU_DEBUG(sql_string);
#endif

    if (!m_impl || !m_impl->m_db) {
        co_await connect();
    }

    auto exec_func = [this, &sql_string]() -> std::tuple<int, int64_t, std::string> {
        std::lock_guard<std::mutex> lock(m_impl->m_db_mutex);
        int rc = sqlite3_exec(m_impl->m_db, sql_string.c_str(), NULL, NULL, NULL);
        int64_t affect_rows = (rc == SQLITE_OK) ? sqlite3_changes(m_impl->m_db) : 0;
        // The error message must be read while the mutex is still held
        return {rc, affect_rows, rc != SQLITE_OK ? sqlite3_errmsg(m_impl->m_db) : ""};
    };

    auto [rc, affect_rows, errmsg] = co_await co_run(m_impl->m_thread_pool->executor(), exec_func);

    SQL_CHECK(rc == SQLITE_OK, rc, "SQL error: {}! ({})", errmsg, sql_string);

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
        auto st = co_await getStatement("select count(1) from sqlite_master where name=?");
        st->bind(0, tablename);
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
    int64_t count = co_await queryNumber<int64_t>(
      fmt::format("select count(1) from {}", sqlIdentifier(tablename)));
    SQL_CHECK(count == 0, -1, "The ID cannot be reset when data is present in table({})",
              tablename);
    auto seq_stmt = co_await getStatement("UPDATE sqlite_sequence SET seq=0 WHERE name=?");
    seq_stmt->bind(0, tablename);
    co_await seq_stmt->exec();
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
        std::lock_guard<std::mutex> lock(m_impl->m_db_mutex);
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

    bool result = co_await co_run(m_impl->m_thread_pool->executor(), check_func);
    co_return result;
}

net::awaitable<bool> AsyncSQLiteConnect::backup(const char *zFilename, int n_page, int step_sleep) {
    if (!m_impl || !m_impl->m_db) {
        co_await connect();
    }

    auto backup_func = [this, zFilename, n_page, step_sleep]() -> bool {
        std::lock_guard<std::mutex> lock(m_impl->m_db_mutex);
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

        sqlite3_close_v2(pFile);
        return rc == SQLITE_OK;
    };

    bool result = co_await co_run(m_impl->m_thread_pool->executor(), backup_func);
    co_return result;
}

}  // namespace hku
