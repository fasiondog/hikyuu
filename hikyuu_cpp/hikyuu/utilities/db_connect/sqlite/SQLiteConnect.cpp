/*
 * SQLiteConnect.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-1
 *      Author: fasiondog
 */

#include <thread>
#include "hikyuu/utilities/config.h"
#include "hikyuu/utilities/Log.h"
#include "SQLiteConnect.h"
#include "SQLiteStatement.h"

namespace hku {

// sqlite3 多线程处理时，等待其他锁释放回调处理
static int sqlite_busy_call_back(void *ptr, int count) {
    std::this_thread::yield();
    return 1;
}

SQLiteConnect::SQLiteConnect(const Parameter &param) : DBConnectBase(param), m_db(nullptr) {
    try {
        m_dbname = getParam<std::string>("db");
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
        // 多线程模式下，不同数据库连接不能使用 SQLITE_OPEN_SHAREDCACHE
        // 将导致 table is locked!
        //| SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;
        if (haveParam("flags")) {
            flags = getParam<int>("flags");
        }

        int rc = sqlite3_open_v2(m_dbname.c_str(), &m_db, flags, NULL);
        SQL_CHECK(rc == SQLITE_OK, rc, "{}", sqlite3_errmsg(m_db));

#if HKU_ENABLE_SQLCIPHER
        if (haveParam("key")) {
            std::string key = getParam<std::string>("key");
            if (!key.empty()) {
                rc = sqlite3_key(m_db, key.c_str(), static_cast<int>(key.size()));
                SQL_CHECK(rc == SQLITE_OK, rc, sqlite3_errmsg(m_db));
            }
        }
#endif

        sqlite3_busy_handler(m_db, sqlite_busy_call_back, (void *)m_db);
        if (sqlite3_libversion_number() >= 3003008) {
            sqlite3_extended_result_codes(m_db, true);
        }

    } catch (std::out_of_range &e) {
        HKU_FATAL("Can't get database name! {}", e.what());
        close();
        throw;
    } catch (SQLException &e) {
        HKU_FATAL("Failed open database: {})! SQLite3 errcode: {}, errmsg: {}", m_dbname,
                  e.errcode(), e.what());
        close();
        throw;
    } catch (std::exception &e) {
        HKU_FATAL("Failed initialize data driver({})! exception: {}", m_dbname, e.what());
        close();
        throw;
    } catch (...) {
        HKU_FATAL("Failed open database({})! Unkown error!", m_dbname);
        close();
        throw;
    }
}

SQLiteConnect::~SQLiteConnect() {
    close();
}

bool SQLiteConnect::ping() {
    HKU_IF_RETURN(!m_db, false);

    // sqlite打开时并不会对文件是否是有效sqlite文件进行检查，
    // 只有执行 sql 语句时，才会报 SQLITE_NOTADB(26) 错误
    int rc = sqlite3_exec(m_db, "PRAGMA synchronous;", NULL, NULL, NULL);
    return rc == SQLITE_OK;
}

void SQLiteConnect::close() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

int64_t SQLiteConnect::exec(const std::string &sql_string) {
#if HKU_SQL_TRACE
    HKU_DEBUG(sql_string);
#endif
    int rc = sqlite3_exec(m_db, sql_string.c_str(), NULL, NULL, NULL);
    SQL_CHECK(rc == SQLITE_OK, rc, "SQL error: {}! ({})", sqlite3_errmsg(m_db), sql_string);
    int affect_rows = sqlite3_changes(m_db);
    return affect_rows < 0 ? 0 : affect_rows;
}

void SQLiteConnect::transaction() {
    exec("BEGIN IMMEDIATE");
}

void SQLiteConnect::commit() {
    exec("COMMIT TRANSACTION");
}

void SQLiteConnect::rollback() noexcept {
    try {
        exec("ROLLBACK TRANSACTION");
    } catch (const std::exception &e) {
        HKU_ERROR("Failed rollback! {}", e.what());
    } catch (...) {
        HKU_ERROR("Unknown error!");
    }
}

SQLStatementPtr SQLiteConnect::getStatement(const std::string &sql_statement) {
    return std::make_shared<SQLiteStatement>(this, sql_statement);
}

bool SQLiteConnect::tableExist(const std::string &tablename) {
    SQLStatementPtr st =
      getStatement(fmt::format("select count(1) from sqlite_master where name='{}'", tablename));
    st->exec();
    bool result = false;
    if (st->moveNext()) {
        int tmp;
        st->getColumn(0, tmp);
        if (tmp == 1) {
            result = true;
        }
    }
    return result;
}

void SQLiteConnect::resetAutoIncrement(const std::string &tablename) {
    int64_t count = queryNumber<int64_t>(fmt::format("select count(1) from {}", tablename));
    HKU_CHECK(count == 0, "The ID cannot be reset when data is present in table({})", tablename);
    exec(fmt::format("UPDATE sqlite_sequence SET seq=0 WHERE name='{}'", tablename));
}

bool SQLiteConnect::check(bool quick) noexcept {
    bool good = false;
    sqlite3_stmt *integrity = NULL;
    std::string check_pragma(quick ? "PRAGMA quick_check;" : "PRAGMA integrity_check;");
    if (sqlite3_prepare_v2(m_db, check_pragma.c_str(), -1, &integrity, NULL) == SQLITE_OK) {
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
}

bool SQLiteConnect::backup(const char *zFilename, int n_page, int step_sleep) noexcept {
    sqlite3 *pFile;
    int rc = sqlite3_open(zFilename, &pFile);
    if (rc == SQLITE_OK) {
        /* Open the sqlite3_backup object used to accomplish the transfer */
        sqlite3_backup *pBackup = sqlite3_backup_init(pFile, "main", m_db, "main");
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
}

} /* namespace hku */
