/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-29
 *      Author: fasiondog
 */

#include "../test_config.h"

#if ENABLE_WITH_PLUGIN_TEST

#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/plugin/extind.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator_talib/ta_crt.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/ALIGN.h>

using namespace hku;

/**
 * @defgroup test_indicator_RANK test_indicator_RANK
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_RANK") {
    auto k = getKData("sz000001", KQuery(-30));
    Indicator ret;

    Block blk;
    blk.add(getStock("sz000001"));
    blk.add(getStock("sz000002"));
    ret = RANK(blk, CLOSE())(k);

    for (size_t i = 0; i < ret.size(); ++i) {
        HKU_INFO("{}: {}", i, ret[i]);
    }
    HKU_INFO("{}", ret);

    HKU_INFO("=============================================");
    auto k2 = getKData("sz000002", KQuery(-30));
    ret = RANK(blk, CLOSE())(k2);
    for (size_t i = 0; i < ret.size(); ++i) {
        HKU_INFO("{}: {}", i, ret[i]);
    }
    HKU_INFO("{}", ret);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_RANK_export") {
    // StockManager& sm = StockManager::instance();
    // string filename(sm.tmpdir());
    // filename += "/RANK.xml";

    // Stock stock = sm.getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(-20));
    // Indicator x1 = hku::RANK(MA(CLOSE(kdata), 3));
    // {
    //     std::ofstream ofs(filename);
    //     boost::archive::xml_oarchive oa(ofs);
    //     oa << BOOST_SERIALIZATION_NVP(x1);
    // }

    // Indicator x2;
    // {
    //     std::ifstream ifs(filename);
    //     boost::archive::xml_iarchive ia(ifs);
    //     ia >> BOOST_SERIALIZATION_NVP(x2);
    // }

    // CHECK_UNARY(x1.size() == x2.size());
    // CHECK_UNARY(x1.discard() == x2.discard());
    // CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    // for (size_t i = 0; i < x1.size(); ++i) {
    //     CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    // }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

#endif /** #if ENABLE_WITH_PLUGIN_TEST */

/** @} */
