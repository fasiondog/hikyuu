/*
 * test_SIGNED_POWER.cpp
 *
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026年6月9日
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SIGNED_POWER.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_SIGNED_POWER test_indicator_SIGNED_POWER
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点：测试基本功能和符号保留特性 */
TEST_CASE("test_SIGNED_POWER") {
    Indicator result;

    // 测试正数、负数和0的组合
    PriceList a;
    a.push_back(-2);
    a.push_back(-1);
    a.push_back(0);
    a.push_back(1);
    a.push_back(2);

    Indicator data = PRICELIST(a);

    // 测试3次方 - 验证符号保留
    result = SIGNED_POWER(data, 3);
    CHECK_EQ(result.name(), "SIGNED_POWER");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 5);
    // (-2)^3 = -8，保留负号
    CHECK_EQ(result[0], doctest::Approx(-8.0));
    // (-1)^3 = -1，保留负号
    CHECK_EQ(result[1], doctest::Approx(-1.0));
    // 0^3 = 0
    CHECK_EQ(result[2], doctest::Approx(0.0));
    // 1^3 = 1
    CHECK_EQ(result[3], doctest::Approx(1.0));
    // 2^3 = 8
    CHECK_EQ(result[4], doctest::Approx(8.0));

    // 测试2次方 - 验证符号保留（负数平方应为负数的平方，但保留符号）
    result = SIGNED_POWER(data, 2);
    CHECK_EQ(result.name(), "SIGNED_POWER");
    CHECK_EQ(result.discard(), 0);
    // (-2)^2 = -4（保留负号）
    CHECK_EQ(result[0], doctest::Approx(-4.0));
    // (-1)^2 = -1（保留负号）
    CHECK_EQ(result[1], doctest::Approx(-1.0));
    // 0^2 = 0
    CHECK_EQ(result[2], doctest::Approx(0.0));
    // 1^2 = 1
    CHECK_EQ(result[3], doctest::Approx(1.0));
    // 2^2 = 4
    CHECK_EQ(result[4], doctest::Approx(4.0));

    // 测试单个数值
    result = SIGNED_POWER(-11, 3);
    CHECK_EQ(result.name(), "SIGNED_POWER");
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    // -11^3 = -1331，保留负号
    CHECK_EQ(result[0], doctest::Approx(-1331.0));

    result = SIGNED_POWER(5, 3);
    CHECK_EQ(result[0], doctest::Approx(125.0));
}

/** @par 检测点：测试动态参数版本 */
TEST_CASE("test_SIGNED_POWER_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    Indicator c = CLOSE(kdata);

    // 使用整数参数作为参考
    Indicator expect = SIGNED_POWER(c, 3);

    // 测试使用CVAL作为动态参数
    Indicator result = SIGNED_POWER(c, CVAL(c, 3));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    // 测试使用IndParam包装的动态参数
    result = SIGNED_POWER(c, IndParam(CVAL(c, 3)));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SIGNED_POWER_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SIGNED_POWER.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SIGNED_POWER(CLOSE(kdata), 3);
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

    CHECK_EQ(x1.name(), x2.name());
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */