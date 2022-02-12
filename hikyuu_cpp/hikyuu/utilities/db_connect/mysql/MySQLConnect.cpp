/*
 * MySQLConnect.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-8-17
 *      Author: fasiondog
 */

#include "MySQLConnect.h"

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
        SQL_CHECK(mysql_real_connect(m_mysql, host.c_str(), usr.c_str(), pwd.c_str(),
                                     database.c_str(), port, NULL, CLIENT_MULTI_STATEMENTS) != NULL,
                  mysql_errno(m_mysql), "Failed to connect to database! {}", mysql_error(m_mysql));
        SQL_CHECK(mysql_set_character_set(m_mysql, "utf8") == 0, mysql_errno(m_mysql),
                  "mysql_set_character_set error! {}", mysql_error(m_mysql));

    } catch (std::bad_alloc& e) {
        HKU_THROW("Failed allocate MySQLConnect! {}", e.what());

    } catch (hku::exception&) {
        close();
        throw;

    } catch (std::exception& e) {
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

void MySQLConnect::exec(const string& sql_string) {
    int ret = mysql_query(m_mysql, sql_string.c_str());
    if (ret) {
        // 尝试重新连接
        if (ping()) {
            ret = mysql_query(m_mysql, sql_string.c_str());
        } else {
            SQL_THROW(ret, "SQL error：{}! error code：{}, error msg: {}", sql_string, ret,
                      mysql_error(m_mysql));
        }
    }

    if (ret) {
        SQL_THROW(ret, "SQL error：{}! error code：{}, error msg: {}", sql_string, ret,
                  mysql_error(m_mysql));
    }

    do {
        MYSQL_RES* result = mysql_store_result(m_mysql);
        if (result) {
            // auto num_fields = mysql_num_fields(result);
            // HKU_TRACE("num_fields: {}", num_fields);
            mysql_num_fields(result);
            mysql_free_result(result);
        } else {
            ret = mysql_field_count(m_mysql);
            if (ret == 0) {
#if defined(_DEBUG) || defined(DEBUG)
                auto num_rows = mysql_affected_rows(m_mysql);
                HKU_TRACE("num_rows: {}", num_rows);
#endif
            } else {
                SQL_THROW(ret, "mysql_field_count error：{}! error code：{}, error msg: {}",
                          sql_string, ret, mysql_error(m_mysql));
            }
        }
    } while (!mysql_next_result(m_mysql));
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

void MySQLConnect::transaction() {
    exec("BEGIN");
}

void MySQLConnect::commit() {
    exec("COMMIT");
}

void MySQLConnect::rollback() {
    exec("ROLLBACK");
}

}  // namespace hku
