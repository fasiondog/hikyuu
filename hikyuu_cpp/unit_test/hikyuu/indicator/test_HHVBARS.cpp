/*
 * test_HHVBARS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019年4月1日
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/HHVBARS.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/REF.h>

using namespace hku;

/**
 * @defgroup test_indicator_HHVBARS test_indicator_HHVBARS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_HHVBARS") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("sh000001");
    KData k = stk.getKData(-10);
    Indicator c = CLOSE(k);

    Indicator result;

    /** @arg n = 0, 高点顺序上升 */
    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);
    result = HHVBARS(data, 0);
    CHECK_EQ(result.name(), "HHVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0);
    }

    /** @arg n = 0, 高点降序 */
    a.clear();
    for (int i = 0; i < 10; ++i) {
        a.push_back(10 - i);
    }

    data = PRICELIST(a);
    result = HHVBARS(data, 0);
    CHECK_EQ(result.name(), "HHVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], i);
    }

    /** @arg n = 0， 顺序随机 */
    result = HHVBARS(c, 0);
    CHECK_EQ(result.name(), "HHVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 1);
    CHECK_EQ(result[3], 2);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 2);
    CHECK_EQ(result[9], 5);

    /** @arg n = 1 */
    result = HHVBARS(c, 1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], 0);
    }

    /** @arg n = 5 */
    result = HHVBARS(c, 5);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 1);
    CHECK_EQ(result[3], 2);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[8], 4);
    CHECK_EQ(result[9], 3);

    /** @arg n = 10 */
    result = HHVBARS(c, 10);
    CHECK_EQ(result.name(), "HHVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 2);
    CHECK_EQ(result[9], 5);

    /** @arg ind.size() == 1 */
    result = HHVBARS(CVAL(1));
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    /** @arg n > ind.size() */
    result = HHVBARS(c, 20);
    CHECK_GT(20, c.size());
    CHECK_EQ(result.name(), "HHVBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 1);
    CHECK_EQ(result[3], 2);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 2);
    CHECK_EQ(result[9], 5);
}

/** @par 检测点 */
TEST_CASE("test_HHVBARS_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = HHVBARS(c, 10);
    Indicator result = HHVBARS(c, CVAL(c, 10));
    CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = HHVBARS(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = HHVBARS(c, 0);
    result = HHVBARS(c, IndParam(CVAL(c, 0)));
    CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

/** @par 检测点 */
TEST_CASE("test_HHVBARS_dyn2") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    auto XGTS = HHVBARS(HIGH(), 20);
    auto XDJ = REF(LOW(), XGTS);

    auto result = XDJ(kdata);

    // auto x = PRICELIST(XGTS(kdata));
    auto x = XGTS(kdata);
    auto expect = REF(LOW(kdata), x);
    check_indicator(result, expect);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_HHVBARS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/HHVBARS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = HHVBARS(CLOSE(kdata), 2);
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

    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
