/*
 * MySQLConnect.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#include "MySQLConnect.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

namespace hku {

MySQLConnect::MySQLConnect(const Parameter& param) : DBConnectBase(param), m_mysql(nullptr) {
    try {
        m_mysql = new MYSQL;
        HKU_CHECK(mysql_init(m_mysql) != NULL, "Initial MySQL handle error!");

        string host = getParamFromOther<string>(param, "host", "127.0.0.1");
        string usr = getParamFromOther<string>(param, "usr", "root");
        string pwd = getParamFromOther<string>(param, "pwd", "");
        string database = getParamFromOther<string>(param, "db", "");
        unsigned int port = getParamFromOther<int>(param, "port", 3306);
        HKU_TRACE("MYSQL host: {}", host);
        HKU_TRACE("MYSQL port: {}", port);
        HKU_TRACE("MYSQL database: {}", database);

        my_bool reconnect = 1;
        SQL_CHECK(mysql_options(m_mysql, MYSQL_OPT_RECONNECT, &reconnect) == 0,
                  mysql_errno(m_mysql), "Failed set reconnect options, {}", mysql_error(m_mysql));

        // 20220314: 新版 mysqlclient 默认 ssl 可能被开启，这里强制设为关闭
        unsigned int ssl_mode = SSL_MODE_DISABLED;
        SQL_CHECK(mysql_options(m_mysql, MYSQL_OPT_SSL_MODE, &ssl_mode) == 0, mysql_errno(m_mysql),
                  "Failed set ssl_mode options, {}", mysql_error(m_mysql));

        SQL_CHECK(mysql_real_connect(m_mysql, host.c_str(), usr.c_str(), pwd.c_str(),
                                     database.c_str(), port, NULL, CLIENT_MULTI_STATEMENTS) != NULL,
                  mysql_errno(m_mysql), "Failed to connect to database! {}", mysql_error(m_mysql));
        SQL_CHECK(mysql_set_character_set(m_mysql, "utf8") == 0, mysql_errno(m_mysql),
                  "mysql_set_character_set error! {}", mysql_error(m_mysql));

    } catch (const std::bad_alloc& e) {
        HKU_THROW("Failed allocate MySQLConnect! {}", e.what());

    } catch (const hku::exception& e) {
        close();
        HKU_THROW("Failed create MySQLConnect! {}", e.what());

    } catch (const std::exception& e) {
        close();
        HKU_THROW("Failed create MySQLConnent instance! {}", e.what());

    } catch (...) {
        close();
        HKU_THROW("Failed create MySQLConnect instance! Unknown error");
    }
}

MySQLConnect::~MySQLConnect() {
    close();
}

void MySQLConnect::close() {
    if (m_mysql) {
        mysql_close(m_mysql);
        delete m_mysql;
        m_mysql = nullptr;
    }
}

bool MySQLConnect::ping() {
    HKU_IF_RETURN(!m_mysql, false);
    auto ret = mysql_ping(m_mysql);
    HKU_ERROR_IF_RETURN(ret, false, "mysql_ping error code: {}, msg: {}", ret,
                        mysql_error(m_mysql));
    return true;
}

int64_t MySQLConnect::exec(const std::string& sql_string) {
#ifdef HKU_SQL_TRACE
    HKU_DEBUG(sql_string);
#endif
    int ret = mysql_query(m_mysql, sql_string.c_str());
    if (ret) {
        // 尝试重新连接
        if (ping()) {
            ret = mysql_query(m_mysql, sql_string.c_str());
        } else {
            SQL_THROW(ret, "SQL error：{}! error msg: {}", sql_string, mysql_error(m_mysql));
        }
    }

    if (ret) {
        SQL_THROW(ret, "SQL error：{}! error msg: {}", sql_string, mysql_error(m_mysql));
    }

    int64_t affect_rows = mysql_affected_rows(m_mysql);
    if (affect_rows == (my_ulonglong)-1) {
        affect_rows = 0;
    }

    do {
        MYSQL_RES* result = mysql_store_result(m_mysql);
        if (result) {
            // auto num_fields = mysql_num_fields(result);
            // HKU_TRACE("num_fields: {}", num_fields);
            mysql_num_fields(result);
            mysql_free_result(result);
        } else {
            if (mysql_field_count(m_mysql) == 0) {
#if defined(_DEBUG) || defined(DEBUG)
                auto num_rows = mysql_affected_rows(m_mysql);
                HKU_TRACE("num_rows: {}", num_rows);
#endif
            } else {
                SQL_THROW(ret, "mysql_field_count error：{}! error msg: {}", sql_string,
                          mysql_error(m_mysql));
            }
        }
    } while (!mysql_next_result(m_mysql));
    return affect_rows;
}

SQLStatementPtr MySQLConnect::getStatement(const string& sql_statement) {
    return make_shared<MySQLStatement>(this, sql_statement);
}

bool MySQLConnect::tableExist(const string& tablename) {
    bool result = false;
    try {
        SQLStatementPtr st = getStatement(fmt::format("SELECT 1 FROM {} LIMIT 1;", tablename));
        st->exec();
        result = true;
    } catch (...) {
        result = false;
    }
    return result;
}

void MySQLConnect::resetAutoIncrement(const std::string& tablename) {
    int64_t count = queryNumber<int64_t>(fmt::format("select count(1) from {}", tablename));
    HKU_CHECK(count == 0, "The ID cannot be reset when data is present in table({})", tablename);
    exec(fmt::format("alter {} auto_increment=1", tablename));
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
        HKU_ERROR("rollback failed! {}", e.what());
    } catch (...) {
        HKU_ERROR("Unknonw error!");
    }
}

}  // namespace hku

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
