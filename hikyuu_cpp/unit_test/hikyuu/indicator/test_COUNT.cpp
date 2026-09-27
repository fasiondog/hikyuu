/*
 * test_LIUTONGPAN.cpp
 *
 *  Created on: 2019-3-29
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/COUNT.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/REF.h>

using namespace hku;

/**
 * @defgroup test_indicator_COUNT test_indicator_COUNT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_COUNT") {
    /** @arg n > 0 */
    Indicator C = CLOSE();
    Indicator x = COUNT(C > REF(C, 1), 5);
    x.setContext(getStock("sh600004"), KQuery(-8));
    CHECK_EQ(x.size(), 8);
    CHECK_EQ(x.discard(), 5);
    CHECK_EQ(x[5], 3);
    CHECK_EQ(x[6], 2);
    CHECK_EQ(x[7], 2);
    /*C.setContext(getStock("sh600004"), KQuery(-8));
    for (int i = 1; i < 8 ; i++) {
        std::cout << i << " " << C[i-1] << " " << C[i] << " " << x[i] << std::endl;
    }*/

    /** @arg n == 0 */
    x = COUNT(C > REF(C, 1), 0);
    x.setContext(getStock("sh600004"), KQuery(-8));
    CHECK_EQ(x.size(), 8);
    CHECK_EQ(x.discard(), 1);
    CHECK_UNARY(std::isnan(x[0]));
    CHECK_EQ(x[1], 1);
    CHECK_EQ(x[2], 2);
    CHECK_EQ(x[3], 2);
    CHECK_EQ(x[4], 3);
    CHECK_EQ(x[5], 3);
    CHECK_EQ(x[6], 3);
    CHECK_EQ(x[7], 4);
}

/** @par Test points */
TEST_CASE("test_COUNT_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = COUNT(c, 10);
    Indicator result = COUNT(c, CVAL(c, 10));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = COUNT(c, IndParam(CVAL(c, 10)));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = COUNT(c, 0);
    result = COUNT(c, IndParam(CVAL(c, 0)));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

/**
 * @par Test points
 * A NaN in the condition series means no data: it neither satisfies nor breaks the condition,
 * consistent with IBarsSince and with every consumer (SG_Bool, IF, conditions all treat NaN as
 * not holding).
 */
TEST_CASE("test_COUNT_nan_is_not_a_condition") {
    PriceList a;
    a.push_back(0);
    a.push_back(0);
    a.push_back(Null<price_t>());
    a.push_back(0);
    a.push_back(0);

    Indicator x = PRICELIST(a);
    Indicator result = COUNT(x, 3);
    CHECK_EQ(result.size(), 5);
    // the first two bars have no full window
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    for (size_t i = result.discard(); i < result.size(); ++i) {
        CHECK_EQ(result[i], 0);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_COUNT_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/COUNT.xml";

    Indicator C = CLOSE();
    Indicator x1 = COUNT(C > REF(C, 1), 5);
    x1.setContext(getStock("sh600004"), KQuery(-8));

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
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
