/*
 * test_mysql.cpp
 *
 *  Created on: 2020-9-26
 *      Author: fasiondog
 */

#include "doctest/doctest.h"

#include <hikyuu/config.h>

#if HKU_ENABLE_MYSQL_KDATA

#include <hikyuu/StockManager.h>
#include <hikyuu/utilities/ConnectPool.h>
#include <hikyuu/utilities/db_connect/DBConnect.h>
#include <hikyuu/utilities/db_connect/mysql/MySQLConnect.h>

using namespace hku;

#define HOST "127.0.0.1"
#define PASSWORD ""

#if 0
TEST_CASE("test_mysql") {
    Parameter param;
    param.set<string>("host", HOST);
    param.set<int>("port", 3306);
    param.set<string>("usr", "root");
    param.set<string>("pwd", PASSWORD);
    auto con = std::make_shared<MySQLConnect>(param);
    CHECK(con->ping());

    con->exec(format("create database if not exists test; use test;"));
    CHECK(con->tableExist("t2018") == false);
    con->exec("create table t2018 (name VARCHAR(20), age INT)");
    CHECK(con->tableExist("t2018") == true);
    con->exec("drop table t2018");
    CHECK(con->tableExist("t2018") == false);

    {
        if (!con->tableExist("t2019")) {
            con->exec(
              "create table t2019 (id INTEGER PRIMARY KEY AUTO_INCREMENT, "
              "name VARCHAR(50), "
              "data_int32_t INT, data_int64_t BIGINT, data_double DOUBLE, data_float FLOAT)");
        }

        class T2019 {
            TABLE_BIND5(t2019, name, data_int32_t, data_int64_t, data_double, data_float);

        public:
            T2019()
            : name(Null<string>()),
              data_int32_t(Null<int32_t>()),
              data_int64_t(Null<int64_t>()),
              data_double(Null<double>()),
              data_float(Null<float>()) {}

            void reset() {
                name = "";
                data_int32_t = Null<int32_t>();
                data_int64_t = Null<int64_t>();
                data_double = Null<double>();
                data_float = Null<float>();
            }

        public:
            string name;
            int32_t data_int32_t;
            int64_t data_int64_t;
            double data_double;
            float data_float;
        };

        T2019 x;
        x.name = "Davis";
        x.data_int32_t = 32;
        x.data_int64_t = 3147483647;
        x.data_double = 3.1415926;
        x.data_float = 3.14f;
        con->save(x);

        T2019 rx;
        con->load(rx);

        CHECK(rx.name == x.name);
        CHECK(rx.data_int32_t == x.data_int32_t);
        CHECK(rx.data_int64_t == x.data_int64_t);
        CHECK(std::abs(rx.data_double - x.data_double) < 0.00001);
        CHECK(std::abs(rx.data_float - x.data_float) < 0.00001);
        con->exec("drop table t2019");
    }

    {
        class TTT {
            TABLE_BIND4(ttt, name, age, email, other)
        public:
            TTT() {}
            TTT(const string& name, int age) : name(name), age(age) {}
            TTT(const string& name, int age, const string& email)
            : name(name), age(age), email(email) {}
            string name;
            int age;
            string email;
            string other;

            string toString() {
                return fmt::format("ttt(id: {}, name: {}, age: {}, email: {}, other: {})", id(),
                                   name, age, email, other);
            }
        };

        con->exec(
          "create table if not exists ttt (id INTEGER PRIMARY KEY AUTO_INCREMENT, name "
          "VARCHAR(50), age INT, email VARCHAR(50), other VARCHAR(50))");

        vector<TTT> t_list;
        t_list.push_back(TTT("aaa", 20, "aaa@x.com"));
        t_list.push_back(TTT("bbb", 30, "bbb@x.com"));
        t_list.push_back(TTT("ccc", 15, "ccc@x.com"));
        con->batchSave(t_list.begin(), t_list.end());
        /*for (auto& r : t_list) {
            HKU_INFO("{}", r.toString());
        }*/

        vector<TTT> r_list;
        con->batchLoad(r_list, "1=1 order by name DESC");
        /*for (auto& r : r_list) {
            HKU_INFO("{}", r.toString());
        }*/

        CHECK(r_list.size() == 3);
        CHECK(r_list[0].name == "ccc");
        CHECK(r_list[0].age == 15);
        CHECK(r_list[0].email == "ccc@x.com");
        CHECK(r_list[1].name == "bbb");
        CHECK(r_list[1].age == 30);
        CHECK(r_list[1].email == "bbb@x.com");
        CHECK(r_list[2].name == "aaa");
        CHECK(r_list[2].age == 20);
        CHECK(r_list[2].email == "aaa@x.com");

        TTT x;
        con->load(x, "name='bbb'");
        x.age = 100;
        con->save(x);

        TTT y;
        con->load(y, "name='bbb'");
        CHECK(y.age == 100);

        con->exec("drop table ttt");
    }

    {
        con->exec(
          "create table if not exists perf_test (id INTEGER PRIMARY KEY AUTO_INCREMENT, name "
          "CHAR(10), value DOUBLE)");

        class PerformancTest {
            TABLE_BIND2(perf_test, name, value)

        public:
            PerformancTest(const string& name, double value) : name(name), value(value) {}

        public:
            string name;
            double value;
        };

        vector<PerformancTest> t_list;
        size_t total = 10000;
        for (auto i = 0; i < total; i++) {
            t_list.push_back(PerformancTest(std::to_string(i), i));
        }
        {
            SPEND_TIME_MSG(batch, "insert mysql, total records: {}", total);
            con->batchSave(t_list.begin(), t_list.end());
        }
        con->exec("drop table perf_test");
    }

    con->exec("drop database if exists test;");
}
#endif

#endif  // HKU_ENABLE_MYSQL_KDATA