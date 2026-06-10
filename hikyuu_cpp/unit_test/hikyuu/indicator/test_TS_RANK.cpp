/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2026-06-09
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
 * @defgroup test_indicator_TS_RANK test_indicator_TS_RANK
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点：TS_RANK基础功能测试 */
TEST_CASE("test_TS_RANK_base") {
    Indicator ind = PRICELIST({5.0, 3.0, 8.0, 3.0, 2.0, 9.0, 1.0, 7.0, 4.0, 6.0}, 0);

    Indicator ret = TS_RANK(ind, 5);

    CHECK_EQ(ret.name(), "TS_RANK");
    CHECK_EQ(ret.size(), 10);
    CHECK_EQ(ret.discard(), 4);

    price_t nan = Null<price_t>();
    Indicator expect = PRICELIST({nan, nan, nan, nan, 0.2, 1.0, 0.2, 0.8, 0.6, 0.6}, 4);
    check_indicator(ret, expect);
}

/** @par 检测点：TS_RANK参数为1时的行为 */
TEST_CASE("test_TS_RANK_n1") {
    Indicator ind = PRICELIST({1.0, 2.0, 3.0, 4.0, 5.0});

    Indicator ret = TS_RANK(ind, 1);

    CHECK_EQ(ret.size(), 5);
    CHECK_EQ(ret.discard(), 0);

    Indicator expect = PRICELIST({1.0, 1.0, 1.0, 1.0, 1.0});
    check_indicator(ret, expect);
}

/** @par 检测点：TS_RANK处理NaN值 */
TEST_CASE("test_TS_RANK_with_nan") {
    price_t nan = Null<price_t>();
    Indicator ind = PRICELIST({nan, 3.0, 5.0, 4.0, 6.0, 2.0}, 1);

    Indicator ret = TS_RANK(ind, 3);

    CHECK_EQ(ret.size(), 6);
    CHECK_EQ(ret.discard(), 3);
}

/** @par 检测点：TS_RANK空数据处理 */
TEST_CASE("test_TS_RANK_empty") {
    Indicator ind;

    Indicator ret = TS_RANK(ind, 5);

    CHECK_EQ(ret.size(), 0);
    CHECK_EQ(ret.discard(), 0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点：TS_RANK序列化 */
TEST_CASE("test_TS_RANK_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TS_RANK.xml";

    Indicator ind = PRICELIST({5.0, 3.0, 8.0, 3.0, 2.0, 9.0, 1.0, 7.0, 4.0, 6.0}, 0);
    Indicator x1 = TS_RANK(ind, 5);
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
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */