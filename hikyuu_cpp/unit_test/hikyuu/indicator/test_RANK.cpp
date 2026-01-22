/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-29
 *      Author: fasiondog
 */

#include "../test_config.h"

#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/plugin/extind.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/plugin/interface/plugins.h>
#include <hikyuu/plugin/device.h>
#include "../plugin_valid.h"

using namespace hku;

/**
 * @defgroup test_indicator_RANK test_indicator_RANK
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_RANK") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sz000001", KQuery(-5));
    Indicator ret;

    Block blk;
    blk.add(getStock("sz000001"));
    blk.add(getStock("sz000002"));
    ret = RANK(blk, CLOSE())(k);
    CHECK_EQ(ret.name(), "RANK");

    price_t nan = Null<price_t>();
    Indicator expect = PRICELIST({nan, 1, 1, 1, 1}, 1);
    check_indicator(ret, expect);

    auto k2 = getKData("sz000002", KQuery(-6));
    ret = RANK(blk, CLOSE())(k2);
    expect = PRICELIST({2, 2, 2, 2, 2, 1});
    check_indicator(ret, expect);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_RANK_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/RANK.xml";

    Block blk;
    blk.add(getStock("sz000001"));
    blk.add(getStock("sz000002"));
    auto k = getKData("sz000002", KQuery(-6));
    Indicator x1 = RANK(blk, CLOSE())(k);
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

    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
