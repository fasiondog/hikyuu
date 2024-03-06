/*
 * test_SAFTYLOSS.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SAFTYLOSS.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_SAFTYLOSS test_indicator_SAFTYLOSS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SAFTYLOSS") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KQuery query;
    KData kdata;
    Indicator close, result;

    /** @arg 源数据为空 */
    result = SAFTYLOSS(CLOSE(kdata), 2, 1);
    CHECK_EQ(result.empty(), true);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg 参数n1、n2非法 */
    query = KQuery(0, 20);
    kdata = stock.getKData(query);
    close = CLOSE(kdata);
    result = SAFTYLOSS(close, 1, 1);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), close.size());
    for (size_t i = 0; i < result.size(); ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    result = SAFTYLOSS(close, 0, 0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), close.size());

    result = SAFTYLOSS(close, 2, 0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), close.size());

    /** @arg 正常参数 */
    result = SAFTYLOSS(close, 2, 1, 1.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], doctest::Approx(27.67));
    CHECK_EQ(result[2], doctest::Approx(28.05));
    CHECK_EQ(result[3], doctest::Approx(27.45));
    CHECK_LT(std::fabs(result[19] - 25.54), 0.0001);

    result = SAFTYLOSS(close, 2, 2, 1.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_EQ(result[2], doctest::Approx(28.05));
    CHECK_EQ(result[3], doctest::Approx(28.05));
    CHECK_EQ(result[4], doctest::Approx(27.45));
    CHECK_LT(std::fabs(result[19] - 25.54), 0.0001);

    result = SAFTYLOSS(close, 3, 2, 2.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 3);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_LT(std::fabs(result[3] - 27.97), 0.0001);
    CHECK_LT(std::fabs(result[4] - 27.15), 0.0001);
    CHECK_LT(std::fabs(result[5] - 25.05), 0.0001);
    CHECK_LT(std::fabs(result[19] - 24.84), 0.0001);

    result = SAFTYLOSS(close, 3, 3, 2.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 4);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[3]));
    CHECK_LT(std::fabs(result[4] - 27.97), 0.0001);
    CHECK_LT(std::fabs(result[5] - 27.15), 0.0001);
    CHECK_LT(std::fabs(result[6] - 26.7), 0.0001);
    CHECK_LT(std::fabs(result[19] - 25.68), 0.0001);

    result = SAFTYLOSS(close, 10, 3, 2.0);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.size(), close.size());
    CHECK_EQ(result.discard(), 11);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[10]));
    CHECK_LT(std::fabs(result[11] - 25.7486), 0.0001);
    CHECK_LT(std::fabs(result[12] - 25.79), 0.0001);
    CHECK_LT(std::fabs(result[13] - 26.03), 0.0001);
    CHECK_LT(std::fabs(result[19] - 26.105), 0.0001);

    /** @arg operator() */
    Indicator ind = SAFTYLOSS(10, 3, 2.0);
    CHECK_EQ(ind.size(), 0);
    result = ind(close);
    Indicator expect = SAFTYLOSS(close, 10, 3, 2.0);
    CHECK_EQ(result.size(), expect.size());
    CHECK_UNARY(result.size() != 0);
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = result.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

/** @par 检测点 */
TEST_CASE("test_SAFTYLOSS_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-50));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = SAFTYLOSS(c, 10, 3, 2.0);
    Indicator result = SAFTYLOSS(c, CVAL(c, 10), CVAL(c, 3), CVAL(c, 2.0));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
    }

    result = SAFTYLOSS(c, IndParam(CVAL(c, 10)), IndParam(CVAL(c, 3)), IndParam(CVAL(c, 2.0)));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(expect.get(i, 0), doctest::Approx(result.get(i, 0)));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SAFTYLOSS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SAFTYLOSS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = SAFTYLOSS(CLOSE(kdata));
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
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
