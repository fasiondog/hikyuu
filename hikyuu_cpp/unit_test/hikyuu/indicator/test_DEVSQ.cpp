/*
 * test_AMA.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/DEVSQ.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_AMA test_indicator_DEVSQ
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_DEVSQ_nan") {
    double nan = Null<double>();

    /** @arg a window containing NaN outputs NaN (propagation, same semantics as the dynamic path)
     */
    PriceList a;
    for (double v : {1.0, 2.0, nan, 4.0, 5.0, 6.0}) {
        a.push_back(v);
    }
    Indicator result = DEVSQ(PRICELIST(a), 3);
    CHECK_EQ(result.size(), 6);
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[3]));
    CHECK_UNARY(std::isnan(result[4]));

    /** @arg the fully valid window right after the NaN leaving it must still use the real mean
     *  (the rolling update was skipped there and the value-initialized 0.0 mean emitted
     *  4*4+5*5+6*6 = 77 instead of the sum of squared deviations) */
    CHECK_EQ(result[5], doctest::Approx(2.0));

    /** @arg NaN-free windows are exact sums of squared deviations */
    PriceList b;
    for (double v : {1.0, 2.0, 3.0, 4.0}) {
        b.push_back(v);
    }
    Indicator ok = DEVSQ(PRICELIST(b), 3);
    CHECK_EQ(ok[2], doctest::Approx(2.0));  // [1,2,3], mean 2
    CHECK_EQ(ok[3], doctest::Approx(2.0));  // [2,3,4], mean 3

    /** @arg a window containing +-inf also degrades to NaN: the inf element's own deviation
     *  (inf - inf)^2 is NaN; every window is computed independently so it never leaks further */
    double inf = std::numeric_limits<double>::infinity();
    PriceList c;
    for (double v : {1.0, inf, 3.0}) {
        c.push_back(v);
    }
    CHECK_UNARY(std::isnan(DEVSQ(PRICELIST(c), 3)[2]));
    PriceList d;
    for (double v : {inf, -inf, 3.0}) {
        d.push_back(v);
    }
    CHECK_UNARY(std::isnan(DEVSQ(PRICELIST(d), 3)[2]));
}

/** @par Test points */
TEST_CASE("test_DEVSQ_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = DEVSQ(c, 10);
    Indicator result = DEVSQ(c, CVAL(c, 10));
    CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = DEVSQ(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

/** @} */
