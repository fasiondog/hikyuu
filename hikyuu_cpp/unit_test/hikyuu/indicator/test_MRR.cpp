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
#include <hikyuu/indicator/crt/MRR.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MRR test_indicator_MRR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MRR") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, close, mrr;

    /** @arg n = 10 且数据大小刚好为10 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    auto c = kdata.close();
    auto m = MRR(c);
    CHECK_EQ(m.name(), "MRR");
    CHECK_EQ(m.empty(), false);
    CHECK_EQ(m.size(), kdata.size());
    CHECK_EQ(m.discard(), 0);
    std::vector<price_t> expects{0.,      0.103922, 0.103922, 0.118056, 1.35151,
                                 1.35151, 2.29046,  2.29046,  2.29046,  2.29046};
    check_indicator(m, PRICELIST(expects));

    /** @arg n = 0 时，正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    close = CLOSE(kdata);
    mrr = MRR(close, 0);
    CHECK_EQ(mrr.name(), "MRR");
    CHECK_EQ(mrr.empty(), false);
    CHECK_EQ(mrr.size(), kdata.size());
    CHECK_EQ(mrr.discard(), 0);
    check_indicator(mrr, PRICELIST(expects));

    /** @arg n = 1 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-5));
    close = CLOSE(kdata);
    mrr = MRR(close, 1);
    CHECK_EQ(mrr.name(), "MRR");
    CHECK_EQ(mrr.empty(), false);
    CHECK_EQ(mrr.size(), kdata.size());
    CHECK_EQ(mrr.discard(), 0);
    Indicator zero_ind = PRICELIST(PriceList(kdata.size(), 0.0));
    check_indicator(mrr, zero_ind);

    /** @arg 空数据测试 */
    kdata = KData();
    close = CLOSE(kdata);
    mrr = MRR(close, 0);
    CHECK_EQ(mrr.name(), "MRR");
    CHECK_EQ(mrr.size(), 0);
    CHECK_EQ(mrr.empty(), true);

    /** @arg 极端上涨序列 */
    PriceList rising_data{100.0, 110.0, 120.0, 130.0, 140.0};
    Indicator rising = PRICELIST(rising_data);
    Indicator mrr_rising = MRR(rising, 0);
    expects = {0.0, 10.0, 20.0, 30.0, 40.0};
    check_indicator(mrr_rising, PRICELIST(expects));

    mrr_rising = MRR(rising, 3);
    expects = {0.0, 10.0, 20.0, 18.181818, 16.66667};
    check_indicator(mrr_rising, PRICELIST(expects));

    /** @arg 极端下跌序列 */
    PriceList falling_data{100.0, 90.0, 80.0, 70.0, 60.0};
    Indicator falling = PRICELIST(falling_data);
    Indicator mrr_falling = MRR(falling, 0);
    Indicator zero_ind2 = PRICELIST(PriceList(falling_data.size(), 0.0));
    check_indicator(mrr_falling, zero_ind2);

    /** @arg 包含相等价格的情况 */
    PriceList equal_data{100.0, 100.0, 100.0, 100.0, 100.0};
    Indicator equal = PRICELIST(equal_data);
    Indicator mrr_equal = MRR(equal, 0);
    Indicator zero_equal = PRICELIST(PriceList(equal_data.size(), 0.0));
    check_indicator(mrr_equal, zero_equal);

    /** @arg 单一数据点 */
    PriceList single_data{100.0};
    Indicator single = PRICELIST(single_data);
    Indicator mrr_single = MRR(single, 0);
    CHECK_EQ(mrr_single.size(), 1);
    CHECK_EQ(mrr_single[0], 0.0);

    /** @arg 空数据 */
    PriceList empty_data{};
    Indicator empty = PRICELIST(empty_data);
    Indicator mrr_empty = MRR(empty, 0);
    CHECK_EQ(mrr_empty.size(), 0);
    CHECK_EQ(mrr_empty.empty(), true);

    /** @arg 增量计算 */
    kdata = stock.getKData(KQuery(-20, -10));
    m = MRR(CLOSE(), 3)(kdata);
    mrr = m(stock.getKData(-15));
    m = MRR(CLOSE(), 3)(stock.getKData(-15));
    for (size_t i = 10; i < mrr.size(); i++) {
        CHECK_EQ(mrr[i], doctest::Approx(m[i]));
    }

    /** @arg 反例: 先冲高再挖坑, 锁定 look-ahead bias bug (与 IMdd 对称)
     * 原增量算法用窗口全局 min 作盈利基准, 当窗口最低点出现在最高点之后时
     * 高估盈利比率。数据 [1.0, 2.0, 1.5, 0.5, 1.2] n=4:
     *   i=4 窗口 [2.0, 1.5, 0.5, 1.2] 最低 0.5 在最高 2.0 之后,
     *     标准 MRR=140(1.2 相对其前累计 min 0.5), bug 版=300(2.0 相对全局 min 0.5)
     * 全量 n=4<total=5 走分支B(首段变长)+委托 _increment_calculate,
     * 修复后两段均用标准 run_min 基准, 与标准 MRR 一致。
     */
    PriceList mrr_lookahead_data{1.0, 2.0, 1.5, 0.5, 1.2};
    Indicator mrr_lookahead = MRR(PRICELIST(mrr_lookahead_data), 4);
    CHECK_EQ(mrr_lookahead[4], doctest::Approx(140.0).epsilon(0.0001));
}

/** @par 检测点: 全量==增量等价 (复用同一实例连续 setContext 触发 _increment_calculate) */
TEST_CASE("test_MRR_increment_equivalence") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm.getStock("SH600000");
    KData k_full = stk.getKData(KQuery(-30));
    KData k_partial = stk.getKData(KQuery(-30, -15));

    Indicator ind_full = MRR(CLOSE(), 5);
    ind_full.setContext(k_full);

    Indicator ind_inc = MRR(CLOSE(), 5);
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
TEST_CASE("test_MRR_with_nan") {
    // 数据覆盖方案4 内外层防御: i=3(NaN)命中增量外层isnan, i=4(-5)命中外层<=0,
    // i=5 窗口含 j=3(NaN)命中内层isnan, i=6 窗口含 j=4(-5)命中内层<=0
    // n=3<total=7: [0,3)走分支B全量首段, [3,7)委托方案4增量
    PriceList data{1.0, 2.0, 1.5, std::numeric_limits<double>::quiet_NaN(), -5.0, 0.5, 1.2};
    Indicator mrr = MRR(PRICELIST(data), 3);
    CHECK_EQ(mrr.size(), 7);
    // i=3 增量当前点 NaN, 方案4 continue 不写, 保持缓冲初值 NaN
    CHECK(std::isnan(mrr[3]));
    // i=6 窗口[4,6]=[-5,0.5,1.2], 跳过-5后有效[0.5,1.2], run_min=0.5, rr=1.2/0.5-1=140
    CHECK_GE(mrr[6], 0.0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MRR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MRR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator m1 = MRR(kdata.close());
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

    CHECK_EQ(m2.name(), "MRR");
    CHECK_EQ(m1.size(), m2.size());
    CHECK_EQ(m1.discard(), m2.discard());
    CHECK_EQ(m1.getResultNumber(), m2.getResultNumber());
    // 使用check_indicator验证序列化前后的一致性
    check_indicator(m1, m2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */