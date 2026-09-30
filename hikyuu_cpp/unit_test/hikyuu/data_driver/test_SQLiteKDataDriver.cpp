/*
 * test_SQLiteKDataDriver.cpp
 *
 *  Created on: 2026-09-28
 *      Author: hikyuu
 */

#include "hikyuu/utilities/osdef.h"

#if !HKU_OS_WINDOWS

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

// 12 consecutive MIN5 candles (09:35-10:30) with close prices 1..12, converted by the driver
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
        static const int64_t dates[12] = {202401010935, 202401010940, 202401010945, 202401010950,
                                          202401010955, 202401011000, 202401011005, 202401011010,
                                          202401011015, 202401011020, 202401011025, 202401011030};
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

std::vector<Datetime> datetimes(const KRecordList& records) {
    std::vector<Datetime> result;
    result.reserve(records.size());
    for (const auto& record : records) {
        result.push_back(record.datetime);
    }
    return result;
}

// Daily bars with missing trading days: 2024/1/5(Fri), 1/8(Mon)-1/10(Wed), 1/15(Mon)-1/16(Tue),
// 2/1(Thu)-2/2(Fri), close prices 1..8
struct TempDayKDataDB {
    string path;

    TempDayKDataDB() {
        path = (fs::temp_directory_path() / "hku_test_sqlite_kdata_day.sqlite3").string();
        fs::remove(path);

        Parameter conn_param;
        conn_param.set<string>("db", path);
        SQLiteConnect connect(conn_param);
        connect.exec(
          "create table '600000' (date integer primary key, open real not null, "
          "high real not null, low real not null, close real not null, "
          "amount real not null, count real not null)");
        static const int64_t dates[8] = {20240105, 20240108, 20240109, 20240110,
                                         20240115, 20240116, 20240201, 20240202};
        for (int i = 0; i < 8; ++i) {
            connect.exec(fmt::format(
              "insert into '600000' (date, open, high, low, close, amount, count) values "
              "({}, {}, {}, {}, {}, 0, 0)",
              dates[i], i + 1, i + 1, i + 1, i + 1));
        }
    }

    ~TempDayKDataDB() {
        fs::remove(path);
    }

    Parameter driverParam() const {
        Parameter param;
        param.set<string>("type", "sqlite3");
        param.set<bool>("convert", true);
        param.set<string>("SH_DAY", path);
        return param;
    }
};

// MIN5 bars crossing the lunch break (1/5) and with a missing 9:45 bar (1/8),
// close prices: 10, 11, 12, 13, 20, 21, 22
struct TempMinuteSessionDB {
    string path;

    TempMinuteSessionDB() {
        path = (fs::temp_directory_path() / "hku_test_sqlite_kdata_min.sqlite3").string();
        fs::remove(path);

        Parameter conn_param;
        conn_param.set<string>("db", path);
        SQLiteConnect connect(conn_param);
        connect.exec(
          "create table '600000' (date integer primary key, open real not null, "
          "high real not null, low real not null, close real not null, "
          "amount real not null, count real not null)");
        static const int64_t dates[7] = {202401051125, 202401051130, 202401051305, 202401051310,
                                         202401080935, 202401080940, 202401080950};
        static const int closes[7] = {10, 11, 12, 13, 20, 21, 22};
        for (int i = 0; i < 7; ++i) {
            connect.exec(fmt::format(
              "insert into '600000' (date, open, high, low, close, amount, count) values "
              "({}, {}, {}, {}, {}, 0, 0)",
              dates[i], closes[i], closes[i], closes[i], closes[i]));
        }
    }

    ~TempMinuteSessionDB() {
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

}  // namespace

/**
 * Test index queries on a non-base ktype
 * @par Test points
 */
TEST_CASE("test_SQLiteKDataDriver_index_query_non_base_ktype") {
    TempKDataDB db;
    SQLiteKDataDriver driver;
    REQUIRE(driver.init(db.driverParam()));

    /** @arg 12 MIN5 candles within the morning session convert to 4 MIN15 candles */
    CHECK_EQ(driver.getCount(MARKET, CODE, KQuery::MIN15), 4);

    /** @arg index slicing is applied on the converted candles, head-aligned */
    auto first = driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, 2, KQuery::MIN15));
    CHECK_EQ(closePrices(first), std::vector<price_t>{3, 6});
    auto second = driver.getKRecordList(MARKET, CODE, KQueryByIndex(2, 4, KQuery::MIN15));
    CHECK_EQ(closePrices(second), std::vector<price_t>{9, 12});

    /** @arg converted candles are timestamped at the fixed session boundary (interval end) */
    CHECK_EQ(datetimes(second),
             std::vector<Datetime>{Datetime(2024, 1, 1, 10, 15), Datetime(2024, 1, 1, 10, 30)});

    /** @arg a Null end (open-ended query) returns all candles instead of an empty result */
    auto all =
      driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, Null<int64_t>(), KQuery::MIN15));
    CHECK_EQ(closePrices(all), std::vector<price_t>{3, 6, 9, 12});

    /** @arg a range beyond the data is clamped to the available rows */
    auto over_range = driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, 10, KQuery::MIN15));
    CHECK_EQ(closePrices(over_range), std::vector<price_t>{3, 6, 9, 12});

    /** @arg a partially overlapping tail range keeps only the bucket inside it */
    auto tail = driver.getKRecordList(MARKET, CODE, KQueryByIndex(3, 5, KQuery::MIN15));
    CHECK_EQ(closePrices(tail), std::vector<price_t>{12});
}

/**
 * Test daily bars are aggregated into natural calendar phases without a trading calendar
 * @par Test points
 */
TEST_CASE("test_SQLiteKDataDriver_daily_calendar_buckets") {
    TempDayKDataDB db;
    SQLiteKDataDriver driver;
    REQUIRE(driver.init(db.driverParam()));

    /** @arg weekly buckets follow calendar weeks; the holiday-shortened week keeps its bars and
             no phantom bucket is produced */
    CHECK_EQ(driver.getCount(MARKET, CODE, KQuery::WEEK), 4);
    auto weeks =
      driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, Null<int64_t>(), KQuery::WEEK));
    CHECK_EQ(closePrices(weeks), std::vector<price_t>{1, 4, 6, 8});

    /** @arg weekly candle timestamp is the last actual trading day in that week */
    CHECK_EQ(datetimes(weeks), std::vector<Datetime>{Datetime(20240105), Datetime(20240110),
                                                     Datetime(20240116), Datetime(20240202)});

    /** @arg monthly/quarterly buckets follow the natural month/quarter */
    CHECK_EQ(driver.getCount(MARKET, CODE, KQuery::MONTH), 2);
    auto months =
      driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, Null<int64_t>(), KQuery::MONTH));
    CHECK_EQ(closePrices(months), std::vector<price_t>{6, 8});
    CHECK_EQ(datetimes(months), std::vector<Datetime>{Datetime(20240116), Datetime(20240202)});
    CHECK_EQ(driver.getCount(MARKET, CODE, KQuery::QUARTER), 1);

    /** @arg a date query selects converted buckets by their end timestamp */
    auto ranged = driver.getKRecordList(
      MARKET, CODE, KQueryByDate(Datetime(20240108), Datetime(20240117), KQuery::WEEK));
    CHECK_EQ(closePrices(ranged), std::vector<price_t>{4, 6});
}

/**
 * Test minute bars are grouped by A-share session boundaries across the lunch break
 * @par Test points
 */
TEST_CASE("test_SQLiteKDataDriver_minute_session_boundaries") {
    TempMinuteSessionDB db;
    SQLiteKDataDriver driver;
    REQUIRE(driver.init(db.driverParam()));

    /** @arg MIN60 boundaries are 10:30/11:30/14:00/15:00; the lunch break does not merge bars */
    CHECK_EQ(driver.getCount(MARKET, CODE, KQuery::MIN60), 3);
    auto hours =
      driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, Null<int64_t>(), KQuery::MIN60));
    CHECK_EQ(closePrices(hours), std::vector<price_t>{11, 13, 22});
    CHECK_EQ(datetimes(hours),
             std::vector<Datetime>{Datetime(2024, 1, 5, 11, 30), Datetime(2024, 1, 5, 14, 0),
                                   Datetime(2024, 1, 8, 10, 30)});

    /** @arg MIN30 restarts at 13:30 after the lunch break */
    auto half_hours =
      driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, Null<int64_t>(), KQuery::MIN30));
    CHECK_EQ(closePrices(half_hours), std::vector<price_t>{11, 13, 22});
    CHECK_EQ(datetimes(half_hours),
             std::vector<Datetime>{Datetime(2024, 1, 5, 11, 30), Datetime(2024, 1, 5, 13, 30),
                                   Datetime(2024, 1, 8, 10, 0)});

    /** @arg a missing 9:45 bar does not shift later bars into the wrong MIN15 bucket */
    auto quarters =
      driver.getKRecordList(MARKET, CODE, KQueryByIndex(0, Null<int64_t>(), KQuery::MIN15));
    CHECK_EQ(closePrices(quarters), std::vector<price_t>{11, 13, 21, 22});
    CHECK_EQ(datetimes(quarters),
             std::vector<Datetime>{Datetime(2024, 1, 5, 11, 30), Datetime(2024, 1, 5, 13, 15),
                                   Datetime(2024, 1, 8, 9, 45), Datetime(2024, 1, 8, 10, 0)});

    /** @arg a date query on converted minute bars filters by the trading day */
    auto ranged = driver.getKRecordList(
      MARKET, CODE, KQueryByDate(Datetime(2024, 1, 5), Datetime(2024, 1, 6), KQuery::MIN60));
    CHECK_EQ(closePrices(ranged), std::vector<price_t>{11, 13});
}

/**
 * @}
 */

#endif /* !HKU_OS_WINDOWS */
