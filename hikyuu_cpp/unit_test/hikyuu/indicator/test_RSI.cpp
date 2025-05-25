/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-24
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/RSI.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_RSI test_indicator_RSI
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_RSI") {
    auto k = getKData("sh000001", KQuery(-10));

    price_t nan = Null<price_t>();
    auto expect = PRICELIST({nan, 71.33849, 48.28178, 52.84457, 77.14873, 29.67481, 52.49847,
                             44.35698, 37.35631, 35.624438},
                            1);
    Indicator rsi, ret;
    rsi = RSI(CLOSE());
    ret = rsi(k);
    check_indicator(ret, expect);

    rsi = RSI(CLOSE(k));
    ret = rsi(k);
    check_indicator(ret, expect);

    ret = RSI(CLOSE(k))(k);
    check_indicator(ret, expect);

    ret = RSI(CLOSE())(k);
    check_indicator(ret, expect);

    ret = RSI(CLOSE(k));
    check_indicator(ret, expect);
}

/** @} */