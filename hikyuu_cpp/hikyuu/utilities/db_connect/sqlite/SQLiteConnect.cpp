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

// 自动关闭 sqlite3 数据库资源
class Sqlite3Closer {
public:
    void operator()(sqlite3* db) const {
        if (db) {
            sqlite3_close(db);
        }
    }
};

// sqlite3 多线程处理时，等待其他锁释放回调处理
static int sqlite_busy_call_back(void* ptr, int count) {
    std::this_thread::yield();
    return 1;
}

SQLiteConnect::SQLiteConnect(const Parameter& param) : DBConnectBase(param) {
    try {
        m_dbname = getParam<string>("db");
        int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
        // 多线程模式下，不同数据库连接不能使用 SQLITE_OPEN_SHAREDCACHE
        // 将导致 table is locked!
        //| SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE;
        if (haveParam("flags")) {
            flags = getParam<int>("flags");
        }
        sqlite3* db = NULL;
        int rc = sqlite3_open_v2(m_dbname.c_str(), &db, flags, NULL);
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            throw exception(sqlite3_errmsg(db));
        }
        m_db.reset(db, Sqlite3Closer());
        sqlite3_busy_handler(db, sqlite_busy_call_back, (void*)db);

    } catch (std::out_of_range& e) {
        HKU_THROW("Can't get database name! {}", e.what());
    } catch (hku::exception& e) {
        HKU_THROW("Failed open database: {})! SQLite3 error: {}", m_dbname, e.what());
    } catch (std::exception& e) {
        HKU_THROW("Failed initialize data driver({})! exception: {}", m_dbname, e.what());
    } catch (...) {
        HKU_THROW("Failed open database({})! Unkown error!", m_dbname);
    }
}

void SQLiteConnect::exec(const string& sql_string) {
    HKU_ASSERT_M(m_db, "database is not open! {}", m_dbname);
    int rc = sqlite3_exec(m_db.get(), sql_string.c_str(), NULL, NULL, NULL);
    HKU_ASSERT_M(rc == SQLITE_OK, "SQL error: {}! ({})", sqlite3_errmsg(m_db.get()), sql_string);
}

SQLStatementPtr SQLiteConnect::getStatement(const string& sql_statement) {
    return make_shared<SQLiteStatement>(shared_from_this(), sql_statement);
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
    ;
}

} /* namespace hku */
