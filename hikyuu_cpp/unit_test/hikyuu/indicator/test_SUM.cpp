/*
 * test_SUM.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SUM.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/DISCARD.h>

using namespace hku;

/**
 * @defgroup test_indicator_SUM test_indicator_SUM
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SUM") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg n = 0 */
    result = SUM(data, 0);
    CHECK_EQ(result.name(), "SUM");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    price_t sum = 0;
    for (int i = 0; i < 10; ++i) {
        sum += data[i];
        CHECK_EQ(result[i], sum);
    }

    /** @arg n = 1 */
    result = SUM(data, 1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n = 9 */
    result = SUM(data, 9);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 8);
    CHECK_EQ(result[8], 36);
    CHECK_EQ(result[9], 45);

    /** @arg n = 10 */
    result = SUM(data, 10);
    result = SUM(result, 1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 9);
    CHECK_EQ(result[9], 45);

    /** @arg 增量计算 n=0 */
    auto stk = getStock("sz000001");
    auto k1 = stk.getKData(KQuery(-10, -8));
    auto k2 = stk.getKData(KQuery(-9, -6));
    auto k3 = stk.getKData(KQuery(-5));
    result = SUM(CLOSE(), 0)(k1)(k2);
    check_indicator(result, SUM(CLOSE(), 0)(k2));

    /** @arg 增量计算 n=1 */
    result = SUM(CLOSE(), 1)(k1)(k2);
    check_indicator(result, SUM(CLOSE(), 1)(k2));
    result = SUM(CLOSE(), 1)(k1)(k3);
    check_indicator(result, SUM(CLOSE(), 1)(k3));

    /** @arg 增量计算 n=3 */
    result = SUM(CLOSE(), 3)(k1)(k2);
    check_indicator(result, SUM(CLOSE(), 3)(k2));

    // 重叠长度小于 discard
    k1 = stk.getKData(KQuery(-10, -4));
    k3 = stk.getKData(KQuery(-5));
    result = SUM(CLOSE(), 3)(k1)(k3);
    check_indicator(result, SUM(CLOSE(), 3)(k3));

    k3 = stk.getKData(KQuery(-6));
    result = SUM(CLOSE(), 3)(k1);
    HKU_INFO("-----------------------------------------------------------------------------");
    result.setContext(k3);
    HKU_INFO("-----------------------------------------------------------------------------");
    HKU_INFO("{}", result);
    for (size_t i = 0; i < result.size(); i++) {
        HKU_INFO("{}: result: {}", i, result[i]);
        // CHECK_EQ(result[i], doctest::Approx(SUM(CLOSE(), 3)(k4)[i]).epsilon(0.00001));
    }
    result = SUM(CLOSE(), 3)(k3);
    for (size_t i = 0; i < result.size(); i++) {
        HKU_INFO("{}: result: {}", i, result[i]);
    }
    // check_indicator(result, DISCARD(SUM(CLOSE(), 3), 0)(k4));
}

/** @par 检测点 */
TEST_CASE("test_SUM_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = SUM(c, 10);
    Indicator result = SUM(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = SUM(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = SUM(c, 0);
    result = SUM(c, CVAL(c, 0));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SUM_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SUM.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SUM(CLOSE(kdata), 3);
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
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
