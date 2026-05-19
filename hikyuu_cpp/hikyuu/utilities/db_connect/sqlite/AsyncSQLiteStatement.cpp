/*
 * AsyncSQLiteStatement.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2026-05-09
 *      Author: fasiondog
 */

#include "AsyncSQLiteStatement.h"
#include "AsyncSQLiteConnect.h"
#include <sqlite3.h>
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/thread/algorithm.h"

namespace hku {

// Pimpl 实现结构体
struct AsyncSQLiteStatement::Impl {
    sqlite3 *m_db = nullptr;
    sqlite3_stmt *m_stmt = nullptr;
    bool m_needs_reset = false;
    int m_step_status = SQLITE_DONE;
    bool m_at_first_step = true;
    AsyncSQLiteConnect *m_connect = nullptr;  // 持有连接指针以获取线程池执行器

    Impl(AsyncSQLiteConnect *connect, sqlite3 *db, sqlite3_stmt *stmt)
    : m_db(db), m_stmt(stmt), m_connect(connect) {}

    ~Impl() {
        if (m_stmt) {
            sqlite3_finalize(m_stmt);
        }
    }

    void reset() {
        if (m_needs_reset) {
            int status = sqlite3_reset(m_stmt);
            if (status != SQLITE_OK) {
                m_step_status = SQLITE_DONE;
                SQL_THROW(status, "{}", sqlite3_errmsg(m_db));
            }
            m_needs_reset = false;
            m_step_status = SQLITE_DONE;
            m_at_first_step = true;
        }
    }

    // 从连接获取线程池执行器
    ThreadPool::ExecutorWrapper getExecutor() const {
        return m_connect->getThreadPoolExecutor();
    }
};

AsyncSQLiteStatement::AsyncSQLiteStatement(AsyncSQLiteConnect *connect, const std::string &sql)
: AsyncSQLStatementBase(connect, sql), m_impl(nullptr) {
    HKU_CHECK(connect != nullptr, "Invalid AsyncSQLiteConnect");

    // 确保连接已初始化（同步操作）
    connect->_connect();

    // 在构造函数中准备 statement（同步操作，因为只是本地内存操作）
    auto *raw_conn = connect->getRawConnection();
    sqlite3 *db = static_cast<sqlite3 *>(raw_conn);

    sqlite3_stmt *stmt = nullptr;
    int status =
      sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size() + 1), &stmt, nullptr);
    if (status != SQLITE_OK) {
        if (stmt) {
            sqlite3_finalize(stmt);
        }
        SQL_THROW(status, "Failed prepare sql statement: {}! error msg: {}", sql,
                  sqlite3_errmsg(db));
    }

    HKU_CHECK(stmt != nullptr, "Invalid SQL statement: {}", sql);

    m_impl = std::make_unique<Impl>(connect, db, stmt);
}

AsyncSQLiteStatement::~AsyncSQLiteStatement() {
    // m_impl 会自动清理 sqlite3_stmt
}

void AsyncSQLiteStatement::_reset() {
    if (m_impl) {
        m_impl->reset();
    }
}

net::awaitable<void> AsyncSQLiteStatement::sub_exec() {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }

    // 将 reset 和 step 合并到一个 co_run 中
    auto exec_func = [this]() -> int {
        // 1. 重置语句
        if (m_impl->m_needs_reset) {
            int status = sqlite3_reset(m_impl->m_stmt);
            if (status != SQLITE_OK) {
                return status;
            }
            m_impl->m_needs_reset = false;
            m_impl->m_step_status = SQLITE_DONE;
            m_impl->m_at_first_step = true;
        }

        // 2. 执行第一步
        m_impl->m_step_status = sqlite3_step(m_impl->m_stmt);
        m_impl->m_needs_reset = true;

        if (m_impl->m_step_status != SQLITE_DONE && m_impl->m_step_status != SQLITE_ROW) {
            return m_impl->m_step_status;
        }
        return SQLITE_OK;
    };

    int status = co_await co_run(m_impl->getExecutor(), exec_func);

    if (status != SQLITE_OK) {
        SQL_THROW(status, "{}", sqlite3_errmsg(m_impl->m_db));
    }
    co_return;
}

net::awaitable<bool> AsyncSQLiteStatement::sub_moveNext() {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }

    // moveNext 是本地状态检查，可以直接同步返回
    if (m_impl->m_step_status == SQLITE_ROW) {
        if (m_impl->m_at_first_step) {
            m_impl->m_at_first_step = false;
            co_return true;
        } else {
            // 需要执行 sqlite3_step，这是 I/O 操作
            auto step_func = [this]() -> int {
                m_impl->m_step_status = sqlite3_step(m_impl->m_stmt);
                return m_impl->m_step_status;
            };

            int status = co_await co_run(m_impl->getExecutor(), step_func);

            if (status == SQLITE_DONE) {
                co_return false;
            } else if (status == SQLITE_ROW) {
                co_return true;
            } else {
                SQL_THROW(status, "{}", sqlite3_errmsg(m_impl->m_db));
            }
        }
    } else {
        co_return false;
    }
}

uint64_t AsyncSQLiteStatement::sub_getLastRowid() {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    return sqlite3_last_insert_rowid(m_impl->m_db);
}

int AsyncSQLiteStatement::sub_getNumColumns() const {
    if (!m_impl) {
        return 0;
    }
    return (m_impl->m_at_first_step == false) && (m_impl->m_step_status == SQLITE_ROW)
             ? sqlite3_column_count(m_impl->m_stmt)
             : 0;
}

void AsyncSQLiteStatement::sub_bindNull(int idx) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    _reset();
    int status = sqlite3_bind_null(m_impl->m_stmt, idx + 1);
    SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(m_impl->m_db));
}

void AsyncSQLiteStatement::sub_bindInt(int idx, int64_t value) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    _reset();
    int status = sqlite3_bind_int64(m_impl->m_stmt, idx + 1, value);
    SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(m_impl->m_db));
}

void AsyncSQLiteStatement::sub_bindDouble(int idx, double item) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    _reset();
    int status = sqlite3_bind_double(m_impl->m_stmt, idx + 1, item);
    SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(m_impl->m_db));
}

void AsyncSQLiteStatement::sub_bindDatetime(int idx, const Datetime &item) {
    if (item == Null<Datetime>()) {
        sub_bindNull(idx);
    } else {
        sub_bindText(idx, item.str());
    }
}

void AsyncSQLiteStatement::sub_bindText(int idx, const std::string &item) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    _reset();
    int status = sqlite3_bind_text(m_impl->m_stmt, idx + 1, item.c_str(),
                                   static_cast<int>(item.size()), SQLITE_TRANSIENT);
    SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(m_impl->m_db));
}

void AsyncSQLiteStatement::sub_bindText(int idx, const char *item, size_t len) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    _reset();
    int status =
      sqlite3_bind_text(m_impl->m_stmt, idx + 1, item, static_cast<int>(len), SQLITE_TRANSIENT);
    SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(m_impl->m_db));
}

void AsyncSQLiteStatement::sub_bindBlob(int idx, const std::string &item) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    _reset();
    int status = sqlite3_bind_blob(m_impl->m_stmt, idx + 1, item.data(),
                                   static_cast<int>(item.size()), SQLITE_TRANSIENT);
    SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(m_impl->m_db));
}

void AsyncSQLiteStatement::sub_bindBlob(int idx, const std::vector<char> &item) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    _reset();
    int status = sqlite3_bind_blob(m_impl->m_stmt, idx + 1, item.data(),
                                   static_cast<int>(item.size()), SQLITE_TRANSIENT);
    SQL_CHECK(status == SQLITE_OK, status, "{}", sqlite3_errmsg(m_impl->m_db));
}

void AsyncSQLiteStatement::sub_getColumnAsInt64(int idx, int64_t &item) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    item = sqlite3_column_int64(m_impl->m_stmt, idx);
}

void AsyncSQLiteStatement::sub_getColumnAsDouble(int idx, double &item) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    item = sqlite3_column_double(m_impl->m_stmt, idx);
}

void AsyncSQLiteStatement::sub_getColumnAsDatetime(int idx, Datetime &item) {
    std::string date_str;
    sub_getColumnAsText(idx, date_str);
    item = date_str.empty() ? Datetime() : Datetime(date_str);
}

void AsyncSQLiteStatement::sub_getColumnAsText(int idx, std::string &item) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    const char *data = reinterpret_cast<const char *>(sqlite3_column_text(m_impl->m_stmt, idx));
    item = (data != nullptr) ? std::string(data) : std::string();
}

void AsyncSQLiteStatement::sub_getColumnAsBlob(int idx, std::string &item) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    const char *data = static_cast<const char *>(sqlite3_column_blob(m_impl->m_stmt, idx));
    if (data == nullptr) {
        throw null_blob_exception();
    }
    const int size = sqlite3_column_bytes(m_impl->m_stmt, idx);
    item = std::string(data, size);
}

void AsyncSQLiteStatement::sub_getColumnAsBlob(int idx, std::vector<char> &item) {
    if (!m_impl) {
        throw exception("AsyncSQLiteStatement is not initialized");
    }
    const char *data = static_cast<const char *>(sqlite3_column_blob(m_impl->m_stmt, idx));
    if (data == nullptr) {
        throw null_blob_exception();
    }
    const int size = sqlite3_column_bytes(m_impl->m_stmt, idx);
    item.resize(size);
    memcpy(item.data(), data, size);
}

}  // namespace hku
