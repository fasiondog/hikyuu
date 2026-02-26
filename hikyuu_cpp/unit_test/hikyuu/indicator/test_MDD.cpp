/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MDD.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include "../test_config.h"

using namespace hku;

/**
 * @defgroup test_indicator_MDD test_indicator_MDD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MDD_basic") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, close, mdd;

    /** @arg n = 10 且数据大小刚好为10 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    auto c = kdata.close();
    auto m = MDD(c);
    CHECK_EQ(m.name(), "MDD");
    CHECK_EQ(m.empty(), false);
    CHECK_EQ(m.size(), kdata.size());
    CHECK_EQ(m.discard(), 0);
    std::vector<price_t> expects{0.,      0.,      0.72282, 0.72282, 0.72282,
                                 3.27389, 3.27389, 3.27389, 3.28155, 3.58515};
    check_indicator(m, PRICELIST(expects));

    /** @arg n = 0 时，正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    close = CLOSE(kdata);
    mdd = MDD(close, 0);
    CHECK_EQ(mdd.name(), "MDD");
    CHECK_EQ(mdd.empty(), false);
    CHECK_EQ(mdd.size(), kdata.size());
    CHECK_EQ(mdd.discard(), 0);

    check_indicator(mdd, PRICELIST(expects));

    /** @arg n = 5 时, 正常关联数据 */
    mdd = MDD(close, 5);
    CHECK_EQ(mdd.name(), "MDD");
    CHECK_EQ(mdd.empty(), false);
    CHECK_EQ(mdd.size(), kdata.size());
    CHECK_EQ(mdd.discard(), 0);
    expects = {0., 0., 0.72282, 0.72282, 0.72282, 3.27389, 3.27389, 3.27389, 3.28155, 2.55377};
    check_indicator(mdd, PRICELIST(expects));

    for (int i = 0; i < mdd.size(); ++i) {
        HKU_INFO("{}: {} {:<.5f}", i, close[i], mdd[i]);
    }

    /** @arg n = 1 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-5));
    close = CLOSE(kdata);
    mdd = MDD(close, 1);
    CHECK_EQ(mdd.name(), "MDD");
    CHECK_EQ(mdd.empty(), false);
    CHECK_EQ(mdd.size(), kdata.size());
    CHECK_EQ(mdd.discard(), 0);
    // 使用check_indicator验证所有值都为0
    Indicator zero_ind = PRICELIST(PriceList(kdata.size(), 0.0));
    check_indicator(mdd, zero_ind);

    /** @arg 空数据测试 */
    kdata = KData();
    close = CLOSE(kdata);
    mdd = MDD(close, 0);
    CHECK_EQ(mdd.name(), "MDD");
    CHECK_EQ(mdd.size(), 0);
    CHECK_EQ(mdd.empty(), true);

    /** @arg 自定义数据测试 */
    PriceList data{100.0, 105.0, 102.0, 108.0, 95.0};
    Indicator d = PRICELIST(data);
    Indicator mdd1 = MDD(d, 5);
    CHECK_EQ(mdd1.discard(), 0);
    CHECK_EQ(mdd1[0], 0.0);
    CHECK_EQ(mdd1[1], 0.0);
    CHECK(mdd1[2] > 2.85);  // (102-105)/105*100
    CHECK(mdd1[2] < 2.86);
    CHECK_EQ(mdd1[3], doctest::Approx(mdd1[2]).epsilon(0.001));
    CHECK(mdd1[4] > 12.03);  // (95-108)/108*100
    CHECK(mdd1[4] < 12.04);

    /** @arg operator() 测试 */
    mdd = MDD(5);
    mdd1 = MDD(d, 5);
    Indicator mdd2 = mdd(d);
    CHECK_EQ(mdd.size(), 0);
    CHECK_EQ(mdd1.size(), 5);
    CHECK_EQ(mdd1.size(), mdd2.size());
    // 使用check_indicator验证operator()结果一致性
    check_indicator(mdd1, mdd2);
}

/** @par 检测点 */
TEST_CASE("test_MDD_edge_cases") {
    /** @arg 极端上涨序列 */
    PriceList rising_data{100.0, 110.0, 120.0, 130.0, 140.0};
    Indicator rising = PRICELIST(rising_data);
    Indicator mdd_rising = MDD(rising, 0);
    // 使用check_indicator验证所有值都为0
    Indicator zero_ind = PRICELIST(PriceList(rising_data.size(), 0.0));
    check_indicator(mdd_rising, zero_ind);

    /** @arg 极端下跌序列 */
    PriceList falling_data{100.0, 90.0, 80.0, 70.0, 60.0};
    Indicator falling = PRICELIST(falling_data);
    Indicator mdd_falling = MDD(falling, 0);
    CHECK_EQ(mdd_falling[0], 0.0);
    CHECK(mdd_falling[1] > 9.9);
    CHECK(mdd_falling[1] < 10.1);
    CHECK(mdd_falling[2] > 19.9);
    CHECK(mdd_falling[2] < 20.1);
    CHECK(mdd_falling[3] > 29.9);
    CHECK(mdd_falling[3] < 30.1);
    CHECK(mdd_falling[4] > 39.9);
    CHECK(mdd_falling[4] < 40.1);

    /** @arg 包含相等价格的情况 */
    PriceList equal_data{100.0, 100.0, 100.0, 100.0, 100.0};
    Indicator equal = PRICELIST(equal_data);
    Indicator mdd_equal = MDD(equal, 0);
    // 使用check_indicator验证所有值都为0
    Indicator zero_equal = PRICELIST(PriceList(equal_data.size(), 0.0));
    check_indicator(mdd_equal, zero_equal);

    /** @arg 单一数据点 */
    PriceList single_data{100.0};
    Indicator single = PRICELIST(single_data);
    Indicator mdd_single = MDD(single, 0);
    CHECK_EQ(mdd_single.size(), 1);
    CHECK_EQ(mdd_single[0], 0.0);

    /** @arg 空数据 */
    PriceList empty_data{};
    Indicator empty = PRICELIST(empty_data);
    Indicator mdd_empty = MDD(empty, 0);
    CHECK_EQ(mdd_empty.size(), 0);
    CHECK_EQ(mdd_empty.empty(), true);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MDD_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MDD.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator m1 = MDD(kdata.close());
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(m1);
    }

    Indicator m2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(m2);
    }

    CHECK_EQ(m2.name(), "MDD");
    CHECK_EQ(m1.size(), m2.size());
    CHECK_EQ(m1.discard(), m2.discard());
    CHECK_EQ(m1.getResultNumber(), m2.getResultNumber());
    // 使用check_indicator验证序列化前后的一致性
    check_indicator(m1, m2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
