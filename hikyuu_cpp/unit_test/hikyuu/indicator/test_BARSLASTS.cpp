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
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

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
