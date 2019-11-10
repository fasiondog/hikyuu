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

class MySQLCloser {
public:
    void operator()(MYSQL* db) {
        if (db) {
            mysql_close(db);
        }
    }
};

MySQLConnect::MySQLConnect(const Parameter& param) : DBConnectBase(param) {
    shared_ptr<MYSQL> mysql(new MYSQL, MySQLCloser());
    if (!mysql) {
        HKU_THROW("Can't create MYSQL instance!");
        return;
    }

    string host("127.0.0.1");
    if (haveParam("host")) {
        host = getParam<string>("host");
    } else {
        setParam<string>("host", host);
    }

    string usr("root");
    if (haveParam("usr")) {
        usr = getParam<string>("usr");
    } else {
        setParam<string>("usr", usr);
    }

    string pwd;
    if (haveParam("pwd")) {
        pwd = getParam<string>("pwd");
    } else {
        setParam<string>("pwd", pwd);
    }

    string database("hku_base");
    if (haveParam("db")) {
        database = getParam<string>("db");
    } else {
        setParam<string>("db", database);
    }

    string port_str("3306");
    if (haveParam("port")) {
        port_str = getParam<string>("port");
    } else {
        setParam<string>("port", port_str);
    }

    unsigned int port;
    try {
        port = boost::lexical_cast<unsigned int>(port_str);
    } catch (...) {
        port = 3306;
    }

    // HKU_TRACE("MYSQL host: {}", host);
    // HKU_TRACE("MYSQL port: {}", port);
    // HKU_TRACE("MYSQL database: {}", database);

    HKU_ASSERT_M(mysql_init(mysql.get()) != NULL, "Initial MySQL handle error!");

    my_bool reconnect = 1;
    HKU_ASSERT_M(mysql_options(mysql.get(), MYSQL_OPT_RECONNECT, &reconnect) == 0,
                 "Failed set reconnect options");

    HKU_ASSERT_M(mysql_real_connect(mysql.get(), host.c_str(), usr.c_str(), pwd.c_str(),
                                    database.c_str(), port, NULL, CLIENT_MULTI_STATEMENTS) != NULL,
                 "Failed to connect to database! {}", mysql_error(mysql.get()));

    HKU_ASSERT_M(mysql_set_character_set(mysql.get(), "utf8") == 0,
                 "mysql_set_character_set error!");

    m_mysql = mysql;
    return;
}

MySQLConnect::~MySQLConnect() {}

void MySQLConnect::exec(const string& sql_string) {
    HKU_ASSERT_M(m_mysql, "database is not open!");
    int ret = mysql_query(m_mysql.get(), sql_string.c_str());
    HKU_ASSERT_M(ret == 0, "SQL error： {}! error code：{}", sql_string, ret);
}

SQLStatementPtr MySQLConnect::getStatement(const string& sql_statement) {
    return make_shared<MySQLStatement>(shared_from_this(), sql_statement);
    ;
}

bool MySQLConnect::tableExist(const string& tablename) {
    return false;
}

}  // namespace hku
