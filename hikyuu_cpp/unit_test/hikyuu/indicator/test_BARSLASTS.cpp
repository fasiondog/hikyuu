/*
 * test_BARSLASTS.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: hikyuu
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/BARSLASTS.h>
#include <hikyuu/indicator/crt/BARSLAST.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/REF.h>
#include <hikyuu/indicator/crt/SLICE.h>
#include <hikyuu/indicator/Indicator.h>

using namespace hku;

/**
 * @defgroup test_indicator_BARSLASTS test_BARSLASTS
 * @ingroup test_hikyuu_indicator
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_BARSLASTS") {
    /** @arg n=1时，结果应与BARSLAST一致 */
    PriceList a;
    for (int i = 0; i < 8; ++i) {
        a.push_back(i % 4 == 0 ? 1.0 : 0.0);
    }
    Indicator data = PRICELIST(a);
    Indicator result = BARSLASTS(data, 1);
    Indicator expected = BARSLAST(data);
    CHECK_EQ(result.size(), expected.size());
    CHECK_EQ(result.discard(), expected.discard());
    check_indicator(result, expected);

    /** @arg n=2时的基本测试 */
    result = BARSLASTS(data, 2);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 4);

    // 第0个位置：条件第1次成立，不足2次，应为NaN
    CHECK_UNARY(std::isnan(result[0]));
    // 第1-3个位置：条件只成立1次，不足2次，应为NaN
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[3]));
    // 第4个位置：条件第2次成立，距离第1次成立(位置0)为4
    CHECK_EQ(result[4], 4);
    // 第5-6个位置：条件已成立2次，距离第1次成立(位置0)分别为5,6
    CHECK_EQ(result[5], 5);
    CHECK_EQ(result[6], 6);
    // 第7个位置：条件已成立2次，距离第1次成立(位置0)为7
    CHECK_EQ(result[7], 7);

    /** @arg n=3时的测试 */
    result = BARSLASTS(data, 3);
    CHECK_EQ(result.size(), data.size());
    // 前7个位置：条件只成立2次，不足3次，应为NaN
    for (size_t i = 0; i < 7; ++i) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    /** @arg n <= 0时，应返回全NaN序列 */
    result = BARSLASTS(data, 0);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), data.size());

    result = BARSLASTS(data, -1);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), data.size());

    /** @arg 空数据测试 */
    PriceList empty;
    data = PRICELIST(empty);
    result = BARSLASTS(data, 1);
    CHECK_EQ(result.size(), 0);

    /** @arg 全0数据测试 */
    PriceList zeros;
    for (int i = 0; i < 5; ++i) {
        zeros.push_back(0.0);
    }
    data = PRICELIST(zeros);
    result = BARSLASTS(data, 1);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), data.size());
}

/** @par 检测点 - 动态参数测试 */
TEST_CASE("test_BARSLASTS_dyn") {
    /** @arg 动态参数n与静态参数n=2的结果对比 */
    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i % 3 == 0 ? 1.0 : 0.0);
    }
    Indicator data = PRICELIST(a);

    // 静态参数版本
    Indicator expect_static = BARSLASTS(data, 2);

    // 动态参数版本（使用CVAL创建常量指标）
    Indicator result_dyn = BARSLASTS(data, CVAL(data, 2));

    CHECK_EQ(expect_static.size(), result_dyn.size());
    CHECK_EQ(expect_static.discard(), result_dyn.discard());
    for (size_t i = 0; i < result_dyn.size(); ++i) {
        if (std::isnan(expect_static[i])) {
            CHECK_UNARY(std::isnan(result_dyn[i]));
        } else {
            CHECK_EQ(expect_static[i], doctest::Approx(result_dyn[i]));
        }
    }

    /** @arg 动态参数n与IndParam版本对比 */
    Indicator result_indparam = BARSLASTS(data, IndParam(CVAL(data, 2)));
    CHECK_EQ(expect_static.size(), result_indparam.size());
    CHECK_EQ(expect_static.discard(), result_indparam.discard());
    for (size_t i = 0; i < result_indparam.size(); ++i) {
        if (std::isnan(expect_static[i])) {
            CHECK_UNARY(std::isnan(result_indparam[i]));
        } else {
            CHECK_EQ(expect_static[i], doctest::Approx(result_indparam[i]));
        }
    }

    /** @arg 动态参数变化的情况 */
    // 构造一个变化的n值指标：前半部分n=1，后半部分n=2
    PriceList n_values;
    for (int i = 0; i < 10; ++i) {
        n_values.push_back(i < 5 ? 1.0 : 2.0);
    }
    Indicator n_param = PRICELIST(n_values);

    result_dyn = BARSLASTS(data, n_param);
    CHECK_EQ(result_dyn.size(), data.size());

    // 验证前5个位置使用n=1的逻辑（应该与BARSLAST一致）
    Indicator expect_first_half = BARSLAST(SLICE(data, 0, 5));
    for (size_t i = 0; i < 5; ++i) {
        if (std::isnan(expect_first_half[i])) {
            CHECK_UNARY(std::isnan(result_dyn[i]));
        } else {
            CHECK_EQ(expect_first_half[i], doctest::Approx(result_dyn[i]));
        }
    }

    /** @arg 动态参数n<=0的情况 */
    PriceList zero_n;
    for (int i = 0; i < 10; ++i) {
        zero_n.push_back(0.0);
    }
    Indicator zero_param = PRICELIST(zero_n);
    result_dyn = BARSLASTS(data, zero_param);
    CHECK_EQ(result_dyn.size(), data.size());
    CHECK_EQ(result_dyn.discard(), data.size());

    /** @arg 真实股票数据测试 */
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    Indicator c = CLOSE(kdata);
    Indicator cond = c > REF(c, 1);  // 上涨条件

    // 静态参数
    expect_static = BARSLASTS(cond, 2);

    // 动态参数（常量）
    result_dyn = BARSLASTS(cond, CVAL(cond, 2));
    CHECK_EQ(expect_static.size(), result_dyn.size());
    for (size_t i = 0; i < result_dyn.size(); ++i) {
        if (std::isnan(expect_static[i])) {
            CHECK_UNARY(std::isnan(result_dyn[i]));
        } else {
            CHECK_EQ(expect_static[i], doctest::Approx(result_dyn[i]));
        }
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_BARSLASTS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BARSLASTS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = BARSLASTS(CLOSE(kdata), 2);
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

    CHECK_EQ(x2.name(), "BARSLASTS");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    check_indicator(x1, x2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
