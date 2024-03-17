/*
 * test_IKData.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/RECOVER.h>

using namespace hku;

/**
 * @defgroup test_indicator_RECOVER test_indicator_RECOVER
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_RECOVER") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sz000001");
    KData kdata;
    KQuery query;

    /** @arg 非法 ind */
    CHECK_THROWS_AS(RECOVER_FORWARD(MA()), std::exception);
    CHECK_THROWS_AS(RECOVER_BACKWARD(MA()), std::exception);
    CHECK_THROWS_AS(RECOVER_EQUAL_FORWARD(MA()), std::exception);
    CHECK_THROWS_AS(RECOVER_EQUAL_BACKWARD(MA()), std::exception);

    /** @arg 日线以下复权 */
    Datetime start = Datetime(199509250000);
    Datetime end = Null<Datetime>();
    query = KQueryByDate(start, end, KQuery::MIN60);
    auto weights = stock.getWeight(start, end);
    REQUIRE(weights.size() > 0);

    kdata = stock.getKData(query);
    REQUIRE(kdata.size() > 0);

    auto query2 = KQueryByDate(start, end, KQuery::MIN60, KQuery::FORWARD);
    auto kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.close().equal(kdata2.close()));
    auto result = RECOVER_FORWARD(kdata.close());
    CHECK_EQ(result.name(), "RECOVER_FORWARD");
    CHECK_EQ(result.size(), kdata.size());
    CHECK_UNARY(result.equal(kdata2.close()));

    query2 = KQueryByDate(start, end, KQuery::MIN60, KQuery::BACKWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.open().equal(kdata2.open()));
    result = RECOVER_BACKWARD(kdata.open());
    CHECK_EQ(result.name(), "RECOVER_BACKWARD");
    CHECK_UNARY(result.equal(kdata2.open()));

    query2 = KQueryByDate(start, end, KQuery::MIN60, KQuery::EQUAL_BACKWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.high().equal(kdata2.high()));
    result = RECOVER_EQUAL_BACKWARD(kdata.high());
    CHECK_EQ(result.name(), "RECOVER_EQUAL_BACKWARD");
    CHECK_UNARY(result.equal(kdata2.high()));

    query2 = KQueryByDate(start, end, KQuery::MIN60, KQuery::EQUAL_FORWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.low().equal(kdata2.low()));
    result = RECOVER_EQUAL_FORWARD(kdata.low());
    CHECK_EQ(result.name(), "RECOVER_EQUAL_FORWARD");
    CHECK_UNARY(result.equal(kdata2.low()));

    /** @arg 日线复权 */
    query = KQueryByDate(start, end, KQuery::DAY);
    kdata = stock.getKData(query);
    REQUIRE(kdata.size() > 0);

    query2 = KQueryByDate(start, end, KQuery::DAY, KQuery::FORWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.close().equal(kdata2.close()));
    result = RECOVER_FORWARD(kdata.close());
    CHECK_EQ(result.name(), "RECOVER_FORWARD");
    CHECK_EQ(result.size(), kdata.size());
    CHECK_UNARY(result.equal(kdata2.close()));

    query2 = KQueryByDate(start, end, KQuery::DAY, KQuery::BACKWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.open().equal(kdata2.open()));
    result = RECOVER_BACKWARD(kdata.open());
    CHECK_EQ(result.name(), "RECOVER_BACKWARD");
    CHECK_UNARY(result.equal(kdata2.open()));

    query2 = KQueryByDate(start, end, KQuery::DAY, KQuery::EQUAL_BACKWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.high().equal(kdata2.high()));
    result = RECOVER_EQUAL_BACKWARD(kdata.high());
    CHECK_EQ(result.name(), "RECOVER_EQUAL_BACKWARD");
    CHECK_UNARY(result.equal(kdata2.high()));

    query2 = KQueryByDate(start, end, KQuery::DAY, KQuery::EQUAL_FORWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.low().equal(kdata2.low()));
    result = RECOVER_EQUAL_FORWARD(kdata.low());
    CHECK_EQ(result.name(), "RECOVER_EQUAL_FORWARD");
    CHECK_UNARY(result.equal(kdata2.low()));

    /** @arg 日线以上复权 */
    query = KQueryByDate(start, end, KQuery::WEEK);
    kdata = stock.getKData(query);
    REQUIRE(kdata.size() > 0);

    query2 = KQueryByDate(start, end, KQuery::WEEK, KQuery::FORWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.close().equal(kdata2.close()));
    result = RECOVER_FORWARD(kdata.close());
    CHECK_EQ(result.name(), "RECOVER_FORWARD");
    CHECK_EQ(result.size(), kdata.size());
    CHECK_UNARY(result.equal(kdata2.close()));

    query2 = KQueryByDate(start, end, KQuery::WEEK, KQuery::BACKWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.open().equal(kdata2.open()));
    result = RECOVER_BACKWARD(kdata.open());
    CHECK_EQ(result.name(), "RECOVER_BACKWARD");
    CHECK_UNARY(result.equal(kdata2.open()));

    query2 = KQueryByDate(start, end, KQuery::WEEK, KQuery::EQUAL_BACKWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.high().equal(kdata2.high()));
    result = RECOVER_EQUAL_BACKWARD(kdata.high());
    CHECK_EQ(result.name(), "RECOVER_EQUAL_BACKWARD");
    CHECK_UNARY(result.equal(kdata2.high()));

    query2 = KQueryByDate(start, end, KQuery::WEEK, KQuery::EQUAL_FORWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.low().equal(kdata2.low()));
    result = RECOVER_EQUAL_FORWARD(kdata.low());
    CHECK_EQ(result.name(), "RECOVER_EQUAL_FORWARD");
    CHECK_UNARY(result.equal(kdata2.low()));

    /** @arg 直接以 KData 作为输入参数 */
    query = KQueryByDate(start, end, KQuery::WEEK);
    kdata = stock.getKData(query);
    REQUIRE(kdata.size() > 0);

    query2 = KQueryByDate(start, end, KQuery::WEEK, KQuery::FORWARD);
    kdata2 = stock.getKData(query2);
    REQUIRE(kdata.size() == kdata2.size());
    REQUIRE(!kdata.close().equal(kdata2.close()));
    result = RECOVER_FORWARD(kdata);
    CHECK_EQ(result.name(), "RECOVER_FORWARD");
    CHECK_EQ(result.size(), kdata.size());
    CHECK_UNARY(result.equal(kdata2.close()));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_RECOVER_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/RECOVER.xml";

    Stock stock = sm.getStock("sz000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = RECOVER_BACKWARD(kdata);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(x1);
    }

    Indicator x2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(x2);
    }

    CHECK_EQ(x1.name(), x2.name());
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    CHECK_UNARY(x1.equal(x2));
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
