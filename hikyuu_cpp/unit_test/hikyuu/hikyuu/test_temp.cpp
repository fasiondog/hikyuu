#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>

#include <hikyuu/utilities/ConnectPool.h>
#include <hikyuu/utilities/db_connect/mysql/MySQLConnect.h>

using namespace hku;

TEST_CASE("test_temp") {
    Parameter param;
    param.set<string>("pwd", "");

    ConnectPool<MySQLConnect> pool(param);

    auto con = pool.getConnect();
    CHECK(con->ping());

    con->exec(format("create database if not exists test; use test;"));

    /*if (!con->tableExist("t2019")) {
        con->exec(
          "create table t2019 (id INTEGER PRIMARY KEY AUTOINCREMENT, "
          "name VARCHAR(50), data_blob BLOB, "
          "data_int32 INT, data_int64 BIGINT, data_double DOUBLE, data_float FLOAT)");
    }*/

    // con.exec(format("use mysql;drop database if exists test;"));
}
