/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */
#include "../test_config.h"
#include <fstream>
#include <limits>
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

    /** @arg 反例: 先挖坑再创新高, 锁定 look-ahead bias bug
     * 原增量算法用窗口全局 max 作回撤基准, 当窗口最高点出现在最低点之后时
     * 高估回撤。数据 [1.2,1.1,1.0,0.5,1.5,2.0,1.2,1.0] n=5:
     *   i=6 窗口 [1.0,0.5,1.5,2.0,1.2] 最高 2.0 在最低 0.5 之后,
     *     标准 MDD=50(0.5 相对其前累计 max 1.0), bug 版=75(0.5 相对全局 max 2.0)
     *   i=7 窗口 [0.5,1.5,2.0,1.2,1.0] 同理, 标准=50, bug 版=75
     * 全量计算 n=5<total=8 走分支B(首段变长)+委托 _increment_calculate,
     * 修复后两段均用标准 run_max 基准, 逐点与标准 MDD 一致。
     */
    PriceList lookahead_data{1.2, 1.1, 1.0, 0.5, 1.5, 2.0, 1.2, 1.0};
    Indicator lookahead = PRICELIST(lookahead_data);
    Indicator mdd_lookahead = MDD(lookahead, 5);
    CHECK_EQ(mdd_lookahead[6], doctest::Approx(50.0).epsilon(0.0001));
    CHECK_EQ(mdd_lookahead[7], doctest::Approx(50.0).epsilon(0.0001));
}

/** @par 检测点: 全量==增量等价 (复用同一实例连续 setContext 触发 _increment_calculate) */
TEST_CASE("test_MDD_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("SH600000");
    KData k_full = stk.getKData(KQuery(-30));
    KData k_partial = stk.getKData(KQuery(-30, -15));

    // 全量基准
    Indicator ind_full = MDD(CLOSE(), 5);
    ind_full.setContext(k_full);

    // 增量: 复用同一实例连续 setContext
    Indicator ind_inc = MDD(CLOSE(), 5);
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

/** @par 检测点: 含 NaN/非正数数据防崩溃 (验证方案4 不段错误/除零) */
TEST_CASE("test_MDD_with_nan") {
    // 数据四杀: i=1(NaN)命中外层isnan, i=3(-5)命中外层<=0,
    // i=2窗口含j=1(NaN)命中内层isnan, i=4窗口含j=3(-5)命中内层<=0
    PriceList data{100.0, std::numeric_limits<double>::quiet_NaN(), 105.0, -5.0, 90.0,
                    80.0, 110.0};
    Indicator mdd = MDD(PRICELIST(data), 3);
    CHECK_EQ(mdd.size(), 7);
    // i=1 当前点 NaN, 方案4 不写 dst[1], 保持缓冲初值 NaN
    CHECK(std::isnan(mdd[1]));
    // i=6 窗口[90,80,110]: run_max=90时dd=(90-80)/90=11.1111, 之后110创新高dd=0
    CHECK_EQ(mdd[6], doctest::Approx(11.1111).epsilon(0.0001));
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
