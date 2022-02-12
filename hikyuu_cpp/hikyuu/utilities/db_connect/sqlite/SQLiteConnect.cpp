/*
 * SQLiteConnect.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-1
 *      Author: fasiondog
 */

#include <thread>
#include "../../../config.h"
#include "../../../Log.h"
#include "SQLiteConnect.h"
#include "SQLiteStatement.h"

namespace hku {

// sqlite3 多线程处理时，等待其他锁释放回调处理
static int sqlite_busy_call_back(void* ptr, int count) {
    std::this_thread::yield();
    return 1;
}

SQLiteConnect::SQLiteConnect(const Parameter& param) : DBConnectBase(param), m_db(nullptr) {
    try {
        m_dbname = getParam<string>("db");
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
        // 多线程模式下，不同数据库连接不能使用 SQLITE_OPEN_SHAREDCACHE
        // 将导致 table is locked!
        //| SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;
        if (haveParam("flags")) {
            flags = getParam<int>("flags");
        }
        int rc = sqlite3_open_v2(m_dbname.c_str(), &m_db, flags, NULL);
        SQL_CHECK(rc == SQLITE_OK, rc, sqlite3_errmsg(m_db));
        sqlite3_busy_handler(m_db, sqlite_busy_call_back, (void*)m_db);

    } catch (std::out_of_range& e) {
        HKU_FATAL("Can't get database name! {}", e.what());
        close();
        throw;
    } catch (SQLException& e) {
        HKU_FATAL("Failed open database: {})! SQLite3 errcode: {}, errmsg: {}", m_dbname,
                  e.errcode(), e.what());
        close();
        throw;
    } catch (std::exception& e) {
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

void SQLiteConnect::close() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool SQLiteConnect::ping() {
    return m_db ? true : false;
}

void SQLiteConnect::exec(const string& sql_string) {
    int rc = sqlite3_exec(m_db, sql_string.c_str(), NULL, NULL, NULL);
    SQL_CHECK(rc == SQLITE_OK, rc, "SQL error: {}! ({})", sqlite3_errmsg(m_db), sql_string);
}

SQLStatementPtr SQLiteConnect::getStatement(const string& sql_statement) {
    return make_shared<SQLiteStatement>(this, sql_statement);
}

bool SQLiteConnect::tableExist(const string& tablename) {
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

void SQLiteConnect::transaction() {
    exec("BEGIN IMMEDIATE");
}

void SQLiteConnect::commit() {
    exec("COMMIT TRANSACTION");
}

void SQLiteConnect::rollback() {
    exec("ROLLBACK TRANSACTION");
}

} /* namespace hku */
