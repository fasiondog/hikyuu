/*
 * test_SQLiteBlockInfoDriver.cpp
 *
 *  Created on: 2026-09-28
 *      Author: hikyuu
 */

#include <algorithm>
#include <filesystem>
#include <string>

#include "doctest/doctest.h"

#include "hikyuu/utilities/db_connect/DBConnect.h"
#include "hikyuu/data_driver/block_info/sqlite/SQLiteBlockInfoDriver.h"
#include "hikyuu/utilities/db_connect/sqlite/SQLiteConnect.h"

using namespace hku;

/**
 * @defgroup test_sqlite_block_info_driver_suite test_sqlite_block_info_driver_suite
 * Test SQLiteBlockInfoDriver
 * @{
 */

namespace {

namespace fs = std::filesystem;

const char* const CATEGORY = "test_category";
const char* const OTHER_CATEGORY = "other_category";

// A temporary sqlite database with the block/BlockIndex tables seeded with three blocks:
// test_category/a, test_category/b, other_category/c
struct TempBlockDB {
    string path;

    TempBlockDB() {
        path = (fs::temp_directory_path() / "hku_test_sqlite_block_info.sqlite3").string();
        fs::remove(path);

        Parameter conn_param;
        conn_param.set<string>("db", path);
        SQLiteConnect connect(conn_param);
        connect.exec(
          "create table block (id integer primary key autoincrement, "
          "category text not null, name text not null, market_code text not null)");
        connect.exec(
          "create table BlockIndex (id integer primary key autoincrement, "
          "category text not null, name text not null, market_code text not null)");
        connect.exec(
          "insert into block (category, name, market_code) values "
          "('test_category', 'a', 'sh600001'),"
          "('test_category', 'b', 'sh600002'),"
          "('other_category', 'c', 'sh600003')");
    }

    ~TempBlockDB() {
        fs::remove(path);
    }

    Parameter driverParam() const {
        Parameter param;
        param.set<string>("type", "sqlite3");
        param.set<string>("db", path);
        return param;
    }
};

bool containsCategory(const StringList& categories, const string& category) {
    return std::find(categories.begin(), categories.end(), category) != categories.end();
}

}  // namespace

/**
 * Test remove keeps the in-memory cache consistent with the database
 * @par Test points
 */
TEST_CASE("test_SQLiteBlockInfoDriver_remove") {
    TempBlockDB db;
    SQLiteBlockInfoDriver driver;
    REQUIRE(driver.init(db.driverParam()));
    driver.load();

    /** @arg both categories and the two blocks of the same category are loaded */
    CHECK_UNARY(containsCategory(driver.getAllCategory(), CATEGORY));
    CHECK_UNARY(containsCategory(driver.getAllCategory(), OTHER_CATEGORY));
    CHECK_EQ(driver.getBlockList(CATEGORY).size(), 2);

    /** @arg removing a non-existent block or category is safe and changes nothing */
    CHECK_NOTHROW(driver.remove(CATEGORY, "not_exist"));
    CHECK_NOTHROW(driver.remove("not_exist_category", "a"));
    CHECK_EQ(driver.getBlockList(CATEGORY).size(), 2);
    CHECK_UNARY(containsCategory(driver.getAllCategory(), CATEGORY));

    /** @arg removing one block keeps the other block and the category in the cache */
    driver.remove(CATEGORY, "a");
    CHECK_UNARY(driver.getBlock(CATEGORY, "a").isNull());
    CHECK_UNARY_FALSE(driver.getBlock(CATEGORY, "b").isNull());
    CHECK_EQ(driver.getBlockList(CATEGORY).size(), 1);
    CHECK_UNARY(containsCategory(driver.getAllCategory(), CATEGORY));

    /** @arg the other category is untouched */
    CHECK_UNARY(containsCategory(driver.getAllCategory(), OTHER_CATEGORY));
    CHECK_UNARY_FALSE(driver.getBlock(OTHER_CATEGORY, "c").isNull());

    /** @arg removing the last block removes the category */
    driver.remove(CATEGORY, "b");
    CHECK_EQ(driver.getBlockList(CATEGORY).size(), 0);
    CHECK_UNARY_FALSE(containsCategory(driver.getAllCategory(), CATEGORY));
    CHECK_UNARY(containsCategory(driver.getAllCategory(), OTHER_CATEGORY));
}

/**
 * Test the cache after a reload matches the database: only the target block row is deleted
 * @par Test points
 */
TEST_CASE("test_SQLiteBlockInfoDriver_remove_persistence") {
    TempBlockDB db;

    {
        SQLiteBlockInfoDriver driver;
        REQUIRE(driver.init(db.driverParam()));
        driver.load();
        driver.remove(CATEGORY, "a");
    }

    /** @arg a freshly loaded driver still sees the sibling block and the category */
    SQLiteBlockInfoDriver reloaded;
    REQUIRE(reloaded.init(db.driverParam()));
    reloaded.load();
    CHECK_UNARY(reloaded.getBlock(CATEGORY, "a").isNull());
    CHECK_UNARY_FALSE(reloaded.getBlock(CATEGORY, "b").isNull());
    CHECK_EQ(reloaded.getBlockList(CATEGORY).size(), 1);
    CHECK_UNARY(containsCategory(reloaded.getAllCategory(), CATEGORY));
    CHECK_UNARY(containsCategory(reloaded.getAllCategory(), OTHER_CATEGORY));
}

/**
 * @}
 */
