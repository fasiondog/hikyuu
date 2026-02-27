/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */
#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MDD.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MDD test_indicator_MDD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MDD") {
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

    /** @arg n = 1 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-5));
    close = CLOSE(kdata);
    mdd = MDD(close, 1);
    CHECK_EQ(mdd.name(), "MDD");
    CHECK_EQ(mdd.empty(), false);
    CHECK_EQ(mdd.size(), kdata.size());
    CHECK_EQ(mdd.discard(), 0);
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
    PriceList data{100.0, 105.0, 102.0, 108.0, 95.0, 90.0, 101.0, 77.0};
    Indicator d = PRICELIST(data);
    Indicator mdd1 = MDD(d, 5);
    CHECK_EQ(mdd1.discard(), 0);
    expects = {0.0, 0.0, 2.85714, 2.85714, 12.03704, 16.66667, 16.66667, 28.70370};
    check_indicator(mdd1, PRICELIST(expects));

    /** @arg 极端上涨序列 */
    PriceList rising_data{100.0, 110.0, 120.0, 130.0, 140.0};
    Indicator rising = PRICELIST(rising_data);
    Indicator mdd_rising = MDD(rising, 0);
    // 使用check_indicator验证所有值都为0
    zero_ind = PRICELIST(PriceList(rising_data.size(), 0.0));
    check_indicator(mdd_rising, zero_ind);

    /** @arg 极端下跌序列 */
    PriceList falling_data{100.0, 90.0, 80.0, 70.0, 60.0};
    Indicator falling = PRICELIST(falling_data);
    Indicator mdd_falling = MDD(falling, 0);
    expects = {0.0, 10.0, 20, 30, 40};
    check_indicator(mdd_falling, PRICELIST(expects));

    mdd_falling = MDD(falling, 3);
    expects = {0.0, 10.0, 20., 22.22222, 25.0};
    check_indicator(mdd_falling, PRICELIST(expects));

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

    /** @arg 增量计算 */
    kdata = stock.getKData(KQuery(-20, -10));
    m = MDD(CLOSE(), 3)(kdata);
    mdd1 = m(stock.getKData(-15));
    m = MDD(CLOSE(), 3)(stock.getKData(-15));
    CHECK_EQ(mdd1[0], doctest::Approx(2.4825).epsilon(0.0001));
    CHECK_EQ(mdd1[1], doctest::Approx(2.6372).epsilon(0.0001));
    for (size_t i = 2; i < mdd1.size(); i++) {
        CHECK_EQ(mdd1[i], doctest::Approx(m[i]));
    }
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
