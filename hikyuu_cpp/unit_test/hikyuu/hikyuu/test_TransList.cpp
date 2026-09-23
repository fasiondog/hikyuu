/*
 * test_TransList.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/Stock.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_TransList test_hikyuu_TransList
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_TransList_query_by_index") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    KQuery query;
    TransList result;

    /** @arg start == end > 0*/
    query = KQuery(1, 1);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start == end < 0*/
    query = KQuery(-1, -1);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start == end == 0*/
    query = KQuery(-1, -1);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start > end > 0*/
    query = KQuery(1, 3);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 1, 9, 30, 2), 11.21, 1339, 0));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 1, 9, 30, 5), 11.21, 5263, 0));

    /** @arg start < end < 0*/
    query = KQuery(-3, -1);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));

    /** @arg start < end == 0*/
    query = KQuery(-3, 0);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start and end are not given */
    query = KQuery();
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 8884);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 1, 9, 25, 2), 11.20, 15714, 2));
    CHECK_EQ(result[563], TransRecord(Datetime(2019, 2, 1, 9, 58, 47), 11.04, 91, 0));
    CHECK_EQ(result[8883], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));

    /** @arg end is not given */
    query = KQuery(-3);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));
    CHECK_EQ(result[2], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));

    query = KQuery(8881);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));
    CHECK_EQ(result[2], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));

    /** @arg end equals the total count */
    query = KQuery(8881, 8884);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));
    CHECK_EQ(result[2], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));

    /** @arg end is one less than the total count */
    query = KQuery(8881, 8883);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));

    /** @arg end is greater than the total count */
    query = KQuery(8881, 8885);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));
    CHECK_EQ(result[2], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));

    /** @arg start equals the total count */
    query = KQuery(8884);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start is one less than the total count */
    query = KQuery(8883);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));

    /** @arg start is greater than the total count */
    query = KQuery(8885);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg start>0 and end<0 and they overlap */
    query = KQuery(8881, -1);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));

    /** @arg start<0 and end>0 and they overlap */
    query = KQuery(-3, 8883);
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));
}

/** @par Test points */
TEST_CASE("test_TransList_query_by_date") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    KQuery query;
    TransList result;

    /** @arg start is earlier than the data start date and end is not given */
    query = KQueryByDate(Datetime(201812030000));
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 8884);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 1, 9, 25, 2), 11.20, 15714, 2));
    CHECK_EQ(result[563], TransRecord(Datetime(2019, 2, 1, 9, 58, 47), 11.04, 91, 0));
    CHECK_EQ(result[8883], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));

    /** @arg start equals the data start date and end is not given */
    query = KQueryByDate(Datetime(2019, 2, 1, 9, 25, 2));
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 8884);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 1, 9, 25, 2), 11.20, 15714, 2));
    CHECK_EQ(result[563], TransRecord(Datetime(2019, 2, 1, 9, 58, 47), 11.04, 91, 0));
    CHECK_EQ(result[8883], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));

    /** @arg start is later than the data start date and end is not given */
    query = KQueryByDate(Datetime(2019, 2, 11, 14, 56, 59));
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));
    CHECK_EQ(result[2], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));

    /** @arg start equals end */
    query = KQueryByDate(Datetime(2019, 2, 11, 14, 56, 59), Datetime(2019, 2, 11, 14, 56, 59));
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 0);

    /** @arg Both start and end are inside the data range and end is the last record date */
    query = KQueryByDate(Datetime(2019, 2, 11, 14, 56, 59), Datetime(2019, 2, 11, 15, 0, 0));
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));

    /** @arg start is inside the data range and end is later than the last record date */
    query = KQueryByDate(Datetime(2019, 2, 11, 14, 56, 59), Datetime(201902120000));
    result = stock.getTransList(query);
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(result[0], TransRecord(Datetime(2019, 2, 11, 14, 56, 59), 11.20, 210, 1));
    CHECK_EQ(result[1], TransRecord(Datetime(2019, 2, 11, 14, 57, 2), 11.20, 31, 1));
    CHECK_EQ(result[2], TransRecord(Datetime(2019, 2, 11, 15, 0, 0), 11.21, 5794, 2));
}

/** @} */
