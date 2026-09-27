/*
 * test_SQLiteKDataDriver.cpp
 *
 *  Created on: 2026-09-28
 *      Author: hikyuu
 */

#include <filesystem>
#include <string>
#include <vector>

#include "doctest/doctest.h"

#include "fmt/format.h"

#include "hikyuu/KQuery.h"
#include "hikyuu/KRecord.h"
#include "hikyuu/utilities/Null.h"
#include "hikyuu/data_driver/kdata/sqlite/SQLiteKDataDriver.h"
#include "hikyuu/utilities/db_connect/sqlite/SQLiteConnect.h"

using namespace hku;

/**
 * @defgroup test_sqlite_kdata_driver_suite test_sqlite_kdata_driver_suite
 * Test SQLiteKDataDriver
 * @{
 */

namespace {

namespace fs = std::filesystem;

const char* const MARKET = "SH";
const char* const CODE = "600000";

// 12 consecutive MIN5 candles (09:30-10:25) with close prices 1..12, converted by the driver
// to 4 MIN15 candles whose close prices are 3, 6, 9, 12
struct TempKDataDB {
    string path;

    TempKDataDB() {
        path = (fs::temp_directory_path() / "hku_test_sqlite_kdata.sqlite3").string();
        fs::remove(path);

        Parameter conn_param;
        conn_param.set<string>("db", path);
        SQLiteConnect connect(conn_param);
        connect.exec(
          "create table '600000' (date integer primary key, open real not null, "
          "high real not null, low real not null, close real not null, "
          "amount real not null, count real not null)");
        static const int64_t dates[12] = {202401010930, 202401010935, 202401010940, 202401010945,
                                          202401010950, 202401010955, 202401011000, 202401011005,
                                          202401011010, 202401011015, 202401011020, 202401011025};
        for (int i = 0; i < 12; ++i) {
            connect.exec(fmt::format(
              "insert into '600000' (date, open, high, low, close, amount, count) values "
              "({}, {}, {}, {}, {}, 0, 0)",
              dates[i], i + 1, i + 1, i + 1, i + 1));
        }
    }

    ~TempKDataDB() {
        fs::remove(path);
    }

    Parameter driverParam() const {
        Parameter param;
        param.set<string>("type", "sqlite3");
        param.set<bool>("convert", true);
        param.set<string>("SH_MIN5", path);
        return param;
    }
};

std::vector<price_t> closePrices(const KRecordList& records) {
    std::vector<price_t> result;
    result.reserve(records.size());
    for (const auto& record : records) {
        result.push_back(record.closePrice);
    }
    return result;
}

}  // namespace

/**
 * Test index queries on a non-base ktype are head-aligned with the base ktype
 * @par Test points
 */
TEST_CASE("test_SQLiteKDataDriver_index_query_non_base_ktype") {
    TempKDataDB db;
    SQLiteKDataDriver driver;
    REQUIRE(driver.init(db.driverParam()));

    /** @arg 12 MIN5 candles convert to 4 MIN15 candles (multiplier = 3) */
    CHECK_EQ(driver.getCount(MARKET, CODE, KQuery::MIN15), 4);

    /** @arg the target index i maps to the base range [i*3, (i+1)*3), head-aligned */
    auto first = driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, 2, KQuery::MIN15));
    CHECK_EQ(closePrices(first), std::vector<price_t>{3, 6});
    auto second = driver.getKRecordList(MARKET, CODE, KQueryByIndex(2, 4, KQuery::MIN15));
    CHECK_EQ(closePrices(second), std::vector<price_t>{9, 12});

    /** @arg a Null end (open-ended query) returns all candles instead of an empty result */
    auto all =
      driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, Null<int64_t>(), KQuery::MIN15));
    CHECK_EQ(closePrices(all), std::vector<price_t>{3, 6, 9, 12});

    /** @arg a range beyond the data is clamped to the available rows */
    auto over_range = driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, 10, KQuery::MIN15));
    CHECK_EQ(closePrices(over_range), std::vector<price_t>{3, 6, 9, 12});

    /** @arg a partially overlapping tail range keeps only the complete bucket inside it */
    auto tail = driver.getKRecordList(MARKET, CODE, KQueryByIndex(3, 5, KQuery::MIN15));
    CHECK_EQ(closePrices(tail), std::vector<price_t>{12});
}

/**
 * @}
 */
