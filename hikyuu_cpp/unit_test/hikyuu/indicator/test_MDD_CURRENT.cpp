/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-02
 *      Author: fasiondog
 */
#include "../test_config.h"
#include <fstream>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MDD_CURRENT.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MDD_CURRENT test_indicator_MDD_CURRENT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MDD_CURRENT") {
    /** @arg 基本功能测试 */
    PriceList data{100.0, 95.0, 90.0, 98.0, 105.0, 102.0};
    Indicator d = PRICELIST(data);
    Indicator mdd_current = MDD_CURRENT(d);
    CHECK_EQ(mdd_current.name(), "MDD_CURRENT");
    CHECK_EQ(mdd_current.empty(), false);
    CHECK_EQ(mdd_current.size(), 6);
    CHECK_EQ(mdd_current.discard(), 0);

    // 计算验证：
    // 100 → 历史最高点=100 → 回撤=0%
    // 95 → 历史最高点=100 → 回撤=5%
    // 90 → 历史最高点=100 → 回撤=10%
    // 98 → 历史最高点=100 → 回撤=2%
    // 105 → 历史最高点=105 → 回撤=0%
    // 102 → 历史最高点=105 → 回撤≈2.86%
    std::vector<price_t> expects{0.0, 5.0, 10.0, 2.0, 0.0, 2.857142857142857};
    check_indicator(mdd_current, PRICELIST(expects));

    /** @arg 极端上涨序列 */
    PriceList rising_data{100.0, 110.0, 120.0, 130.0, 140.0};
    Indicator rising = PRICELIST(rising_data);
    Indicator mdd_rising = MDD_CURRENT(rising);
    Indicator zero_ind = PRICELIST(PriceList(rising_data.size(), 0.0));
    check_indicator(mdd_rising, zero_ind);

    /** @arg 极端下跌序列 */
    PriceList falling_data{100.0, 90.0, 80.0, 70.0, 60.0};
    Indicator falling = PRICELIST(falling_data);
    Indicator mdd_falling = MDD_CURRENT(falling);
    expects = {0.0, 10.0, 20.0, 30.0, 40.0};
    check_indicator(mdd_falling, PRICELIST(expects));

    /** @arg 包含相等价格的情况 */
    PriceList equal_data{100.0, 100.0, 100.0, 100.0, 100.0};
    Indicator equal = PRICELIST(equal_data);
    Indicator mdd_equal = MDD_CURRENT(equal);
    Indicator zero_equal = PRICELIST(PriceList(equal_data.size(), 0.0));
    check_indicator(mdd_equal, zero_equal);

    /** @arg 单一数据点 */
    PriceList single_data{100.0};
    Indicator single = PRICELIST(single_data);
    Indicator mdd_single = MDD_CURRENT(single);
    CHECK_EQ(mdd_single.size(), 1);
    CHECK_EQ(mdd_single[0], 0.0);

    /** @arg 空数据 */
    PriceList empty_data{};
    Indicator empty = PRICELIST(empty_data);
    Indicator mdd_empty = MDD_CURRENT(empty);
    CHECK_EQ(mdd_empty.size(), 0);
    CHECK_EQ(mdd_empty.empty(), true);

    /** @arg 波动序列测试 */
    PriceList volatile_data{50.0, 60.0, 55.0, 70.0, 65.0, 80.0, 75.0, 85.0, 80.0, 90.0};
    Indicator volatile_ind = PRICELIST(volatile_data);
    Indicator mdd_volatile = MDD_CURRENT(volatile_ind);
    // 计算验证：
    // 50 → 50 → 0%
    // 60 → 60 → 0%
    // 55 → 60 → 8.33%
    // 70 → 70 → 0%
    // 65 → 70 → 7.14%
    // 80 → 80 → 0%
    // 75 → 80 → 6.25%
    // 85 → 85 → 0%
    // 80 → 85 → 5.88%
    // 90 → 90 → 0%
    expects = {0.0,  0.0, 8.333333333333334, 0.0, 7.142857142857143, 0.0,
               6.25, 0.0, 5.882352941176471, 0.0};
    check_indicator(mdd_volatile, PRICELIST(expects));
}

/** @par 检测点: 含 NaN/负数数据处理 */
TEST_CASE("test_MDD_CURRENT_with_nan") {
    PriceList data{100.0, 105.0, 90.0, std::numeric_limits<double>::quiet_NaN(), -5.0, 90.0, 110.0};
    Indicator mdd = MDD_CURRENT(PRICELIST(data));
    CHECK_EQ(mdd.size(), 7);
    CHECK_EQ(mdd[3], 0.0);                                        // NaN 填0
    CHECK_EQ(mdd[4], doctest::Approx(104.7619).epsilon(0.0001));  // -5，历史最高105 → 104.7619%
    CHECK_EQ(mdd[5], doctest::Approx(14.2857).epsilon(0.0001));   // 90，历史最高105 → 14.2857%
    CHECK_EQ(mdd[6], 0.0);                                        // 110，创历史新高
}

/** @par 检测点: 增量计算 */
TEST_CASE("test_MDD_CURRENT_increment") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));

    // 使用指标链式调用验证增量计算
    Indicator m = MDD_CURRENT(CLOSE())(kdata);
    CHECK_EQ(m.name(), "MDD_CURRENT");
    CHECK_EQ(m.size(), kdata.size());

    // 验证连续调用的增量计算
    KData kdata_more = stock.getKData(KQuery(-25));
    Indicator m_more = m(kdata_more);
    CHECK_EQ(m_more.size(), kdata_more.size());

    // 直接计算对比
    Indicator m_direct = MDD_CURRENT(CLOSE())(kdata_more);
    CHECK_EQ(m_more.size(), m_direct.size());
    for (size_t i = 0; i < m_more.size(); ++i) {
        if (std::isnan(m_more[i]) && std::isnan(m_direct[i])) {
            continue;
        }
        CHECK_EQ(m_more[i], doctest::Approx(m_direct[i]).epsilon(0.0001));
    }
}

/** @par 检测点: 全量==增量等价 (复用同一实例连续 setContext 触发 _increment_calculate) */
TEST_CASE("test_MDD_CURRENT_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("SH600000");
    KData k_full = stk.getKData(KQuery(-30));
    KData k_partial = stk.getKData(KQuery(-30, -15));

    // 全量基准
    Indicator ind_full = MDD_CURRENT(CLOSE());
    ind_full.setContext(k_full);

    // 增量: 复用同一实例连续 setContext
    Indicator ind_inc = MDD_CURRENT(CLOSE());
    ind_inc.setContext(k_partial);
    ind_inc.setContext(k_full);

    CHECK_EQ(ind_full.size(), ind_inc.size());
    for (size_t i = 0; i < ind_full.size(); ++i) {
        if (std::isnan(ind_full[i]) && std::isnan(ind_inc[i])) {
            continue;
        }
        CHECK_EQ(ind_inc[i], doctest::Approx(ind_full[i]).epsilon(0.0001));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MDD_CURRENT_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MDD_CURRENT.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator m1 = MDD_CURRENT(kdata.close());
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

    CHECK_EQ(m2.name(), "MDD_CURRENT");
    CHECK_EQ(m1.size(), m2.size());
    CHECK_EQ(m1.discard(), m2.discard());
    CHECK_EQ(m1.getResultNumber(), m2.getResultNumber());
    check_indicator(m1, m2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */