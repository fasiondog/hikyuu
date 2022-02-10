/*
 * test_AMA.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/AVEDEV.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_AMA test_indicator_AVEDEV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AVEDEV_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = AVEDEV(c, 10);
    Indicator result = AVEDEV(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < expect.size(); i++) {
        if (i >= result.discard()) {
            CHECK(!isnan(result[i]));
        }
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

/** @} */
