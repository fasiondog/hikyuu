/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-12
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MACD.h>
#include <hikyuu/indicator/crt/RESULT.h>

using namespace hku;

/**
 * @defgroup test_indicator_RESULT test_indicator_RESULT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_RESULT") {
    /** @arg 无效参数 */
    CHECK_THROWS_AS(RESULT(-1), std::exception);
    CHECK_THROWS_AS(RESULT(6), std::exception);

    /** @arg 输入空指标 */
    auto ret = RESULT(Indicator(), 0);
    CHECK_EQ(ret.empty(), true);

    /** @arg 正常获取 */
    auto k = getStock("SH000001").getKData(KQuery(-100));
    auto macd = MACD(CLOSE(), 0);
    auto bar = RESULT(macd, 0);
    auto diff = RESULT(macd, 1);
    auto dea = RESULT(macd, 2);
    auto expect = MACD(k.close(), 0);
    CHECK_UNARY(bar(k).equal(expect.getResult(0)));
    CHECK_UNARY(diff(k).equal(expect.getResult(1)));
    CHECK_UNARY(dea(k).equal(expect.getResult(2)));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_RESULT_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/RESULT.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = RESULT(MACD(CLOSE(kdata)), 0);
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
    CHECK_UNARY(ma1.equal(ma2));
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */