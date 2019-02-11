/*
 * test_TimeLine.cpp
 *
 *  Created on: 2019年2月10日
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_base
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/StockManager.h>
#include <hikyuu/Stock.h>

using namespace hku;

/**
 * @defgroup test_hikyuu_TimeLine test_hikyuu_TimeLine
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TimeLine_query_by_index ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    KQuery query;
    TimeLineList result;

    /** @arg start == end > 0*/
    query = KQuery(1, 1);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start == end < 0*/
    query = KQuery(-1, -1);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start == end == 0*/
    query = KQuery(-1, -1);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start > end > 0*/
    query = KQuery(1, 3);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201812030931), 10.57, 26962));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201812030932), 10.53, 15407));

    /** @arg start < end < 0*/
    query = KQuery(-3, -1);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));

    /** @arg start < end == 0*/
    query = KQuery(-3, 0);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg 未指定start, end*/
    query = KQuery();
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 10320);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201812030930), 10.61, 83391));
    BOOST_CHECK(result[6516] == TimeLineRecord(Datetime(201901111006), 10.18, 2823));
    BOOST_CHECK(result[10319] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg 未指定end*/
    query = KQuery(-3);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));
    BOOST_CHECK(result[2] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    query = KQuery(10317);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));
    BOOST_CHECK(result[2] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg end 等于总数*/
    query = KQuery(10317, 10320);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));
    BOOST_CHECK(result[2] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg end 比总数少1*/
    query = KQuery(10317, 10319);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));

    /** @arg end 大于总数*/
    query = KQuery(10317, 10321);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));
    BOOST_CHECK(result[2] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start 等于总数*/
    query = KQuery(10320);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start 比总数少1*/
    query = KQuery(10319);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start 大于总数*/
    query = KQuery(10321);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start>0, end<0, 且两者有交集*/
    query = KQuery(10317, -1);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));

    /** @arg start<0, end>0, 且两者有交集*/
    query = KQuery(-3, 10319);
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TimeLine_query_by_date ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    KQuery query;
    TimeLineList result;

    /** @arg start 小于数据起始日期，未指定end*/
    query = KQueryByDate(Datetime(201812030000));
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 10320);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201812030930), 10.61, 83391));
    BOOST_CHECK(result[6516] == TimeLineRecord(Datetime(201901111006), 10.18, 2823));
    BOOST_CHECK(result[10319] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start 等于数据起始日期，未指定end*/
    query = KQueryByDate(Datetime(201812030930));
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 10320);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201812030930), 10.61, 83391));
    BOOST_CHECK(result[6516] == TimeLineRecord(Datetime(201901111006), 10.18, 2823));
    BOOST_CHECK(result[10319] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start大于数据起始日期，未指定end*/
    query = KQueryByDate(Datetime(201902011457));
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));
    BOOST_CHECK(result[2] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));

    /** @arg start 等于 end*/
    query = KQueryByDate(Datetime(201902011457), Datetime(201902011457));
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start, end 都在数据范围内, 且end为最后一条记录日期 */
    query = KQueryByDate(Datetime(201902011457), Datetime(201902011459));
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));

    /** @arg start 在数据范围内, end大于最后一条记录日期 */
    query = KQueryByDate(Datetime(201902011457), Datetime(201902020000));
    result = stock.getTimeLineList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TimeLineRecord(Datetime(201902011457), 11.20, 46));
    BOOST_CHECK(result[1] == TimeLineRecord(Datetime(201902011458), 11.20, 0));
    BOOST_CHECK(result[2] == TimeLineRecord(Datetime(201902011459), 11.20, 20572));
}

/** @} */


