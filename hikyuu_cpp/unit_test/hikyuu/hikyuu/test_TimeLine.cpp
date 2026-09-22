/*
 * test_TimeLine.cpp
 *
 *  Created on: 2019-2-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/Stock.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_TimeLine test_hikyuu_TimeLine
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_TimeLine_query_by_index") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    KQuery query;
    TimeLineList result;

    /** @arg start == end > 0*/
    query = KQuery(1, 1);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start == end < 0*/
    query = KQuery(-1, -1);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start == end == 0*/
    query = KQuery(-1, -1);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start > end > 0*/
    query = KQuery(1, 3);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201812030931), 10.57, 26962));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201812030932), 10.53, 15407));

    /** @arg start < end < 0*/
    query = KQuery(-3, -1);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));

    /** @arg start < end == 0*/
    query = KQuery(-3, 0);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start and end are not given */
    query = KQuery();
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 10320);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201812030930), 10.61, 83391));
    CHECK_EQ(result[6516], TimeLineRecord(Datetime(201901111006), 10.18, 2823));
    CHECK_EQ(result[10319], TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg end is not given */
    query = KQuery(-3);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));
    CHECK_EQ(result[2], TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    query = KQuery(10317);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));
    CHECK_EQ(result[2], TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg end equals the total count */
    query = KQuery(10317, 10320);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));
    CHECK_EQ(result[2], TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg end is one less than the total count */
    query = KQuery(10317, 10319);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));

    /** @arg end is greater than the total count */
    query = KQuery(10317, 10321);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));
    CHECK_EQ(result[2], TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start equals the total count */
    query = KQuery(10320);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start is one less than the total count */
    query = KQuery(10319);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start is greater than the total count */
    query = KQuery(10321);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start>0 and end<0 and they overlap */
    query = KQuery(10317, -1);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));

    /** @arg start<0 and end>0 and they overlap */
    query = KQuery(-3, 10319);
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));
}

/** @par Test points */
TEST_CASE("test_TimeLine_query_by_date") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    KQuery query;
    TimeLineList result;

    /** @arg start is earlier than the data start date and end is not given */
    query = KQueryByDate(Datetime(201812030000));
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 10320);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201812030930), 10.61, 83391));
    CHECK_EQ(result[6516], TimeLineRecord(Datetime(201901111006), 10.18, 2823));
    CHECK_EQ(result[10319], TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start equals the data start date and end is not given */
    query = KQueryByDate(Datetime(201812030930));
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 10320);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201812030930), 10.61, 83391));
    CHECK_EQ(result[6516], TimeLineRecord(Datetime(201901111006), 10.18, 2823));
    CHECK_EQ(result[10319], TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start is later than the data start date and end is not given */
    query = KQueryByDate(Datetime(201902011457));
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));
    CHECK_EQ(result[2], TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start equals end */
    query = KQueryByDate(Datetime(201902011457), Datetime(201902011457));
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg Both start and end are inside the data range and end is the last record date */
    query = KQueryByDate(Datetime(201902011457), Datetime(201902011459));
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));

    /** @arg start is inside the data range and end is later than the last record date */
    query = KQueryByDate(Datetime(201902011457), Datetime(201902020000));
    result = stock.getTimeLineList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TimeLineRecord(Datetime(201902011457), 11.20, 46));
    CHECK_EQ(result[1], TimeLineRecord(Datetime(201902011458), 11.20, 0));
    CHECK_EQ(result[2], TimeLineRecord(Datetime(201902011459), 11.20, 20572));
}

/** @} */
