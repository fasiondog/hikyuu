/*
 * test_TransList.cpp
 *
 *  Created on: 2019-2-11
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
 * @defgroup test_hikyuu_TransList test_hikyuu_TransList
 * @ingroup test_hikyuu_base_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TransList_query_by_index ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    KQuery query;
    TransList result;

    /** @arg start == end > 0*/
    query = KQuery(1, 1);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start == end < 0*/
    query = KQuery(-1, -1);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start == end == 0*/
    query = KQuery(-1, -1);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start > end > 0*/
    query = KQuery(1, 3);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,1,9,30,2), 11.21, 1339, TransRecord::BUY));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,1,9,30,5), 11.21, 5263, TransRecord::BUY));

    /** @arg start < end < 0*/
    query = KQuery(-3, -1);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));

    /** @arg start < end == 0*/
    query = KQuery(-3, 0);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg 未指定start, end*/
    query = KQuery();
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 8884);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,1,9,25,2), 11.20, 15714, TransRecord::AUCTION));
    BOOST_CHECK(result[563] == TransRecord(Datetime(2019,2,1,9,58,47), 11.04, 91, TransRecord::BUY));
    BOOST_CHECK(result[8883] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));

    /** @arg 未指定end*/
    query = KQuery(-3);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));
    BOOST_CHECK(result[2] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));

    query = KQuery(8881);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));
    BOOST_CHECK(result[2] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));

    /** @arg end 等于总数*/
    query = KQuery(8881, 8884);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));
    BOOST_CHECK(result[2] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));

    /** @arg end 比总数少1*/
    query = KQuery(8881, 8883);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));

    /** @arg end 大于总数*/
    query = KQuery(8881, 8885);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));
    BOOST_CHECK(result[2] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));

    /** @arg start 等于总数*/
    query = KQuery(8884);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start 比总数少1*/
    query = KQuery(8883);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));

    /** @arg start 大于总数*/
    query = KQuery(8885);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 0);


    /** @arg start>0, end<0, 且两者有交集*/
    query = KQuery(8881, -1);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));

    /** @arg start<0, end>0, 且两者有交集*/
    query = KQuery(-3, 8883);
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));
}


/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_TransList_query_by_date ) {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sz000001"];
    KQuery query;
    TransList result;

    /** @arg start 小于数据起始日期，未指定end*/
    query = KQueryByDate(Datetime(201812030000));
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 8884);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,1,9,25,2), 11.20, 15714, TransRecord::AUCTION));
    BOOST_CHECK(result[563] == TransRecord(Datetime(2019,2,1,9,58,47), 11.04, 91, TransRecord::BUY));
    BOOST_CHECK(result[8883] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));

    /** @arg start 等于数据起始日期，未指定end*/
    query = KQueryByDate(Datetime(2019,2,1,9,25,2));
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 8884);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,1,9,25,2), 11.20, 15714, TransRecord::AUCTION));
    BOOST_CHECK(result[563] == TransRecord(Datetime(2019,2,1,9,58,47), 11.04, 91, TransRecord::BUY));
    BOOST_CHECK(result[8883] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));


    /** @arg start大于数据起始日期，未指定end*/
    query = KQueryByDate(Datetime(2019,2,11,14,56,59));
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));
    BOOST_CHECK(result[2] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));

    /** @arg start 等于 end*/
    query = KQueryByDate(Datetime(2019,2,11,14,56,59), Datetime(2019,2,11,14,56,59));
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 0);

    /** @arg start, end 都在数据范围内, 且end为最后一条记录日期 */
    query = KQueryByDate(Datetime(2019,2,11,14,56,59),
                         Datetime(2019,2,11,15,0,0));
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));

    /** @arg start 在数据范围内, end大于最后一条记录日期 */
    query = KQueryByDate(Datetime(2019,2,11,14,56,59),
                         Datetime(201902120000));
    result = stock.getTransList(query);
    BOOST_CHECK(result.size() == 3);
    BOOST_CHECK(result[0] == TransRecord(Datetime(2019,2,11,14,56,59), 11.20, 210, TransRecord::SELL));
    BOOST_CHECK(result[1] == TransRecord(Datetime(2019,2,11,14,57,2), 11.20, 31, TransRecord::SELL));
    BOOST_CHECK(result[2] == TransRecord(Datetime(2019,2,11,15,0,0), 11.21, 5794, TransRecord::AUCTION));
}


/** @} */


