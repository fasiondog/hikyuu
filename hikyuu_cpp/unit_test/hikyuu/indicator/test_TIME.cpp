/*
 * test_IKData.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/TIME.h>

using namespace hku;

/**
 * @defgroup test_indicator_TIME test_indicator_TIME
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TIME") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;

    /** @arg 无输入参数 */
    Indicator date = DATE(), time = TIME(), year = YEAR(), month = MONTH(), week = WEEK(),
              day = DAY(), hour = HOUR(), minute = MINUTE();
    CHECK_EQ(date.size(), 0);
    CHECK_EQ(date.empty(), true);
    CHECK_EQ(date.name(), "DATE");

    CHECK_EQ(time.size(), 0);
    CHECK_EQ(time.empty(), true);
    CHECK_EQ(time.name(), "TIME");

    CHECK_EQ(year.size(), 0);
    CHECK_EQ(year.empty(), true);
    CHECK_EQ(year.name(), "YEAR");

    CHECK_EQ(month.size(), 0);
    CHECK_EQ(month.empty(), true);
    CHECK_EQ(month.name(), "MONTH");

    CHECK_EQ(week.size(), 0);
    CHECK_EQ(week.empty(), true);
    CHECK_EQ(week.name(), "WEEK");

    CHECK_EQ(day.size(), 0);
    CHECK_EQ(day.empty(), true);
    CHECK_EQ(day.name(), "DAY");

    CHECK_EQ(hour.size(), 0);
    CHECK_EQ(hour.empty(), true);
    CHECK_EQ(hour.name(), "HOUR");

    CHECK_EQ(minute.size(), 0);
    CHECK_EQ(minute.empty(), true);
    CHECK_EQ(minute.name(), "MINUTE");

    /** @arg 对应的KData为空 */
    CHECK_EQ(kdata.empty(), true);

    date = DATE(kdata);
    CHECK_EQ(date.size(), 0);
    CHECK_EQ(date.empty(), true);
    CHECK_EQ(date.name(), "DATE");

    time = TIME(kdata);
    CHECK_EQ(time.size(), 0);
    CHECK_EQ(time.empty(), true);
    CHECK_EQ(time.name(), "TIME");

    year = YEAR(kdata);
    CHECK_EQ(year.size(), 0);
    CHECK_EQ(year.empty(), true);
    CHECK_EQ(year.name(), "YEAR");

    month = MONTH(kdata);
    CHECK_EQ(month.size(), 0);
    CHECK_EQ(month.empty(), true);
    CHECK_EQ(month.name(), "MONTH");

    week = WEEK(kdata);
    CHECK_EQ(week.size(), 0);
    CHECK_EQ(week.empty(), true);
    CHECK_EQ(week.name(), "WEEK");

    day = DAY(kdata);
    CHECK_EQ(day.size(), 0);
    CHECK_EQ(day.empty(), true);
    CHECK_EQ(day.name(), "DAY");

    hour = HOUR(kdata);
    CHECK_EQ(hour.size(), 0);
    CHECK_EQ(hour.empty(), true);
    CHECK_EQ(hour.name(), "HOUR");

    minute = MINUTE(kdata);
    CHECK_EQ(minute.size(), 0);
    CHECK_EQ(minute.empty(), true);
    CHECK_EQ(minute.name(), "MINUTE");

    /** @arg 非空的KData */
    KQuery query(-10);
    kdata = stock.getKData(query);
    size_t total = kdata.size();

    date = DATE(kdata);
    CHECK_EQ(date.name(), "DATE");
    CHECK_EQ(date.size(), kdata.size());

    time = TIME(kdata);
    CHECK_EQ(time.name(), "TIME");
    CHECK_EQ(time.size(), kdata.size());

    year = YEAR(kdata);
    CHECK_EQ(year.name(), "YEAR");
    CHECK_EQ(year.size(), kdata.size());

    month = MONTH(kdata);
    CHECK_EQ(month.name(), "MONTH");
    CHECK_EQ(month.size(), kdata.size());

    week = WEEK(kdata);
    CHECK_EQ(week.name(), "WEEK");
    CHECK_EQ(week.size(), kdata.size());

    day = DAY(kdata);
    CHECK_EQ(day.name(), "DAY");
    CHECK_EQ(day.size(), kdata.size());

    hour = HOUR(kdata);
    CHECK_EQ(hour.name(), "HOUR");
    CHECK_EQ(hour.size(), kdata.size());

    minute = MINUTE(kdata);
    CHECK_EQ(minute.name(), "MINUTE");
    CHECK_EQ(minute.size(), kdata.size());

    for (size_t i = 0; i < total; ++i) {
        auto d = kdata[i].datetime;
        CHECK_EQ(date[i], (d.year() - 1900) * 10000 + d.month() * 100 + d.day());
        CHECK_EQ(time[i], d.hour() * 10000 + d.minute() * 100 + d.second());
        CHECK_EQ(year[i], d.year());
        CHECK_EQ(month[i], d.month());
        CHECK_EQ(week[i], d.dayOfWeek());
        CHECK_EQ(day[i], d.day());
        CHECK_EQ(hour[i], d.hour());
        CHECK_EQ(minute[i], d.minute());
    }

    /** @arg 非空的KData */
    date = DATE();
    time = TIME();
    year = YEAR();
    month = MONTH();
    week = WEEK();
    day = DAY();
    hour = HOUR();
    minute = MINUTE();

    date.setContext(stock, query);
    time.setContext(stock, query);
    year.setContext(stock, query);
    month.setContext(stock, query);
    week.setContext(stock, query);
    day.setContext(stock, query);
    hour.setContext(stock, query);
    minute.setContext(stock, query);

    CHECK_EQ(date.name(), "DATE");
    CHECK_EQ(date.size(), kdata.size());

    CHECK_EQ(time.name(), "TIME");
    CHECK_EQ(time.size(), kdata.size());

    CHECK_EQ(year.name(), "YEAR");
    CHECK_EQ(year.size(), kdata.size());

    CHECK_EQ(month.name(), "MONTH");
    CHECK_EQ(month.size(), kdata.size());

    CHECK_EQ(week.name(), "WEEK");
    CHECK_EQ(week.size(), kdata.size());

    CHECK_EQ(day.name(), "DAY");
    CHECK_EQ(day.size(), kdata.size());

    CHECK_EQ(hour.name(), "HOUR");
    CHECK_EQ(hour.size(), kdata.size());

    CHECK_EQ(minute.name(), "MINUTE");
    CHECK_EQ(minute.size(), kdata.size());

    for (size_t i = 0; i < total; ++i) {
        auto d = kdata[i].datetime;
        CHECK_EQ(date[i], (d.year() - 1900) * 10000 + d.month() * 100 + d.day());
        CHECK_EQ(time[i], d.hour() * 10000 + d.minute() * 100 + d.second());
        CHECK_EQ(year[i], d.year());
        CHECK_EQ(month[i], d.month());
        CHECK_EQ(week[i], d.dayOfWeek());
        CHECK_EQ(day[i], d.day());
        CHECK_EQ(hour[i], d.hour());
        CHECK_EQ(minute[i], d.minute());
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TIME_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TIME.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = DATE(kdata);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = 0; i < ma1.size(); ++i) {
        CHECK_EQ(ma1.get(i, 0), doctest::Approx(ma2.get(i, 0)));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
