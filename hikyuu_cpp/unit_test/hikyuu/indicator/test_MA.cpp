/*
 * test_MA.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */
#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_MA test_indicator_MA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MA") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, ma;

    /** @arg n = 3, 但关联数据为空 */
    open = OPEN(kdata);
    ma = MA(open, 3);
    CHECK_EQ(ma.name(), "MA");
    CHECK_EQ(ma.size(), 0);
    CHECK_EQ(ma.empty(), true);

    /** @arg n = 0 时，正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    open = OPEN(kdata);
    ma = MA(open, 0);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 0);
    std::vector<price_t> expects{
      2415.197,  2397.1715, 2395.89,   2392.8908, 2394.1114,
      2396.1477, 2395.6244, 2393.0338, 2389.7090, 2383.4041,
    };
    for (size_t i = 0; i < kdata.size(); ++i) {
        CHECK_EQ(ma[i], doctest::Approx(expects[i]).epsilon(0.0001));
    }

    /** @arg n = 10 且数据大小刚好为10 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    open = OPEN(kdata);
    ma = MA(open, 10);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 9);
    for (size_t i = 0; i < ma.discard(); ++i) {
        CHECK_UNARY(std::isnan(ma[i]));
    }
    CHECK_EQ(ma[9], doctest::Approx(2383.4041));

    /** @arg n = 10 且数据大小刚好为9 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-9));
    open = OPEN(kdata);
    ma = MA(open, 10);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 9);
    for (size_t i = 0; i < ma.discard(); ++i) {
        CHECK_UNARY(std::isnan(ma[i]));
    }

    /** @arg n = 10 且数据大小为11 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-11));
    open = OPEN(kdata);
    ma = MA(open, 10);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 9);
    for (size_t i = 0; i < ma.discard(); ++i) {
        CHECK_UNARY(std::isnan(ma[i]));
    }
    CHECK_EQ(ma[9], doctest::Approx(2390.8365));
    CHECK_EQ(ma[10], doctest::Approx(2383.4041));

    /** @arg n = 1 */
    kdata = stock.getKData(KQuery(-11));
    open = OPEN(kdata);
    ma = MA(open, 1);
    CHECK_EQ(ma.empty(), false);
    CHECK_EQ(ma.size(), kdata.size());
    CHECK_EQ(ma.discard(), 0);
    for (size_t i = 0; i < open.size(); ++i) {
        CHECK_EQ(ma[i], doctest::Approx(open[i]).epsilon(0.001));
    }

    /** @arg 源数据本身带有discard不等于0 */
    PriceList data;
    for (int i = 0; i < 10; ++i) {
        data.push_back(i);
    }

    Indicator d = PRICELIST(data);
    Indicator ma1 = MA(d, 2);
    Indicator ma2 = MA(ma1, 2);
    CHECK_EQ(ma1.discard(), 1);
    CHECK_EQ(ma2.discard(), 2);
    CHECK_UNARY(std::isnan(ma1[0]));
    CHECK_EQ(ma1[1], (data[0] + data[1]) / 2);
    CHECK_EQ(ma1[2], 1.5);
    CHECK_EQ(ma1[3], 2.5);
    CHECK_UNARY(std::isnan(ma2[0]));
    CHECK_UNARY(std::isnan(ma2[1]));
    CHECK_EQ(ma2[2], 1.0);
    CHECK_EQ(ma2[3], 2.0);

    /** @arg operator() */
    ma = MA(2);
    ma1 = MA(d, 2);
    ma2 = ma(d);
    CHECK_EQ(ma.size(), 0);
    CHECK_EQ(ma1.size(), 10);
    CHECK_EQ(ma1.size(), ma2.size());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], ma2[i]);
    }

    /** @arg 增量计算 */
    auto k = stock.getKData(KQuery(-30, -20));
    auto k2 = stock.getKData(KQuery(-30, -10));
    ma = MA(MA(CLOSE(), 2))(k);
    ma2 = MA(MA(CLOSE(), 2))(k2);
    auto ma3 = ma(k2);
    check_indicator(ma3, ma2);

    ma = MA(MA(CLOSE(), 2))(k2);
    ma2 = ma(k);
    ma3 = MA(MA(CLOSE(), 2))(k);
    ma2.setDiscard(ma3.discard());
    check_indicator(ma3, ma2);
}

/** @par 检测点 */
TEST_CASE("test_MA_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = MA(c, 10);
    Indicator result = MA(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = MA(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = MA(c, 0);
    result = MA(c, CVAL(c, 0));
    CHECK_EQ(expect.size(), result.size());
    CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_MA_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    int cycle = 1000;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_MA_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator ind = MA();
            Indicator result = ind(c);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = MA(CLOSE(kdata), 10);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    CHECK_EQ(ma2.name(), "MA");
    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

//----------------------------------------------------------------------------
// NaN 语义测试（Welford 滚动均值，与 IStdev 共用 valid_count 逻辑）
//----------------------------------------------------------------------------

/** @par 检测点：散布 NaN 的滚动均值 */
TEST_CASE("test_MA_nan_scattered") {
    PriceList d;
    d.push_back(1.0);
    d.push_back(2.0);
    d.push_back(3.0);
    d.push_back(Null<price_t>());  // NaN
    d.push_back(5.0);
    d.push_back(6.0);
    d.push_back(7.0);
    d.push_back(8.0);
    d.push_back(9.0);
    d.push_back(10.0);

    Indicator ind = PRICELIST(d);
    Indicator ma = MA(ind, 4);
    CHECK_EQ(ma.discard(), 3);
    CHECK_EQ(ma.size(), 10);
    for (size_t i = 0; i < ma.discard(); ++i) {
        CHECK_UNARY(std::isnan(ma[i]));
    }
    // 窗口 [1,2,3,NaN] → vc=3, mean=2.0
    CHECK_EQ(ma[3], doctest::Approx(2.0).epsilon(0.0001));
    // 窗口 [2,3,NaN,5] → vc=3, mean=10/3
    CHECK_EQ(ma[4], doctest::Approx(3.333333).epsilon(0.0001));
    // 窗口 [3,NaN,5,6] → vc=3, mean=14/3
    CHECK_EQ(ma[5], doctest::Approx(4.666667).epsilon(0.0001));
    // 窗口 [NaN,5,6,7] → vc=3, mean=6.0（NaN 离开后恢复）
    CHECK_EQ(ma[6], doctest::Approx(6.0).epsilon(0.0001));
    // 窗口 [5,6,7,8] → vc=4, mean=6.5（全有效）
    CHECK_EQ(ma[7], doctest::Approx(6.5).epsilon(0.0001));
    CHECK_EQ(ma[8], doctest::Approx(7.5).epsilon(0.0001));
    CHECK_EQ(ma[9], doctest::Approx(8.5).epsilon(0.0001));
}

/** @par 检测点：连续 NaN 穿越后状态恢复 */
TEST_CASE("test_MA_nan_consecutive") {
    PriceList d;
    d.push_back(1.0);
    d.push_back(2.0);
    d.push_back(Null<price_t>());  // NaN
    d.push_back(Null<price_t>());  // NaN
    d.push_back(Null<price_t>());  // NaN
    d.push_back(3.0);
    d.push_back(4.0);

    Indicator ind = PRICELIST(d);
    Indicator ma = MA(ind, 3);
    CHECK_EQ(ma.discard(), 2);
    // i=2 窗口 [1,2,NaN] → vc=2, mean=1.5
    CHECK_EQ(ma[2], doctest::Approx(1.5).epsilon(0.0001));
    // i=3 窗口 [2,NaN,NaN] → vc=1, mean=2.0
    CHECK_EQ(ma[3], doctest::Approx(2.0).epsilon(0.0001));
    // i=4 窗口 [NaN,NaN,NaN] → vc=0, mean=NaN
    CHECK_UNARY(std::isnan(ma[4]));
    // i=5 窗口 [NaN,NaN,3] → vc=1, mean=3.0（从 0 重建）
    CHECK_EQ(ma[5], doctest::Approx(3.0).epsilon(0.0001));
    // i=6 窗口 [NaN,3,4] → vc=2, mean=3.5（恢复）
    CHECK_EQ(ma[6], doctest::Approx(3.5).epsilon(0.0001));
}

/** @par 检测点：窗口仅 1 个有效值（count=1 边界） */
TEST_CASE("test_MA_nan_single_valid") {
    PriceList d;
    for (int i = 0; i < 7; ++i) {
        d.push_back(Null<price_t>());  // NaN
    }
    d[3] = 5.0;  // 唯一有效值

    Indicator ind = PRICELIST(d);
    Indicator ma = MA(ind, 4);
    CHECK_EQ(ma.discard(), 3);
    // vc 恒=1，输出该有效值本身
    CHECK_EQ(ma[3], doctest::Approx(5.0).epsilon(0.0001));
    CHECK_EQ(ma[4], doctest::Approx(5.0).epsilon(0.0001));
    CHECK_EQ(ma[5], doctest::Approx(5.0).epsilon(0.0001));
    CHECK_EQ(ma[6], doctest::Approx(5.0).epsilon(0.0001));
}

/** @par 检测点：大基数小方差（Welford 精度验证） */
TEST_CASE("test_MA_large_base") {
    PriceList d;
    double base = 1e8;
    for (int i = 1; i <= 5; ++i) {
        d.push_back(base + i);
    }

    Indicator ind = PRICELIST(d);
    Indicator ma = MA(ind, 3);
    CHECK_EQ(ma.discard(), 2);
    // 窗口 [1e8+1,1e8+2,1e8+3] → mean=1e8+2
    CHECK_EQ(ma[2], doctest::Approx(100000002.0).epsilon(0.0001));
    CHECK_EQ(ma[3], doctest::Approx(100000003.0).epsilon(0.0001));
    CHECK_EQ(ma[4], doctest::Approx(100000004.0).epsilon(0.0001));
}

/** @par 检测点：极小窗口 n=2 */
TEST_CASE("test_MA_n2") {
    PriceList d;
    d.push_back(3.0);
    d.push_back(Null<price_t>());  // NaN
    d.push_back(5.0);
    d.push_back(7.0);

    Indicator ind = PRICELIST(d);
    Indicator ma = MA(ind, 2);
    CHECK_EQ(ma.discard(), 1);
    // i=1 窗口 [3,NaN] → vc=1, mean=3.0
    CHECK_EQ(ma[1], doctest::Approx(3.0).epsilon(0.0001));
    // i=2 窗口 [NaN,5] → vc=1, mean=5.0
    CHECK_EQ(ma[2], doctest::Approx(5.0).epsilon(0.0001));
    // i=3 窗口 [5,7] → vc=2, mean=6.0
    CHECK_EQ(ma[3], doctest::Approx(6.0).epsilon(0.0001));
}

/** @par 检测点：全 NaN 序列不死循环不抛异常 */
TEST_CASE("test_MA_all_nan") {
    PriceList d;
    for (int i = 0; i < 4; ++i) {
        d.push_back(Null<price_t>());
    }
    Indicator ind = PRICELIST(d);
    Indicator ma = MA(ind, 3);
    CHECK_EQ(ma.size(), 4);
    for (size_t i = 0; i < ma.size(); ++i) {
        CHECK_UNARY(std::isnan(ma[i]));
    }
}

/** @par 检测点：上游 discard 传递 */
TEST_CASE("test_MA_upstream_discard") {
    PriceList d;
    for (int i = 0; i < 10; ++i) {
        d.push_back(i + 1);
    }
    // 构造上游 discard=4：前4个置 NaN
    for (int i = 0; i < 4; ++i) {
        d[i] = Null<price_t>();
    }
    Indicator ind = PRICELIST(d);
    ind.setDiscard(4);  // 显式声明 discard

    Indicator ma = MA(ind, 3);
    // m_discard = 4 + 3 - 1 = 6
    CHECK_EQ(ma.discard(), 6);
    for (size_t i = 0; i < ma.discard(); ++i) {
        CHECK_UNARY(std::isnan(ma[i]));
    }
    // i=6 窗口 [5,6,7] → mean=6.0
    CHECK_EQ(ma[6], doctest::Approx(6.0).epsilon(0.0001));
    CHECK_EQ(ma[7], doctest::Approx(7.0).epsilon(0.0001));
}

/** @par 检测点：_dyn 动态路径与静态 n 等价（含 NaN） */
TEST_CASE("test_MA_dyn_nan_equivalence") {
    PriceList d;
    d.push_back(1.0);
    d.push_back(2.0);
    d.push_back(3.0);
    d.push_back(Null<price_t>());
    d.push_back(5.0);
    d.push_back(6.0);
    d.push_back(7.0);
    d.push_back(8.0);

    Indicator ind = PRICELIST(d);
    Indicator expect = MA(ind, 4);
    Indicator result = MA(ind, CVAL(ind, 4));
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.size(); ++i) {
        if (std::isnan(expect[i])) {
            CHECK_UNARY(std::isnan(result[i]));
        } else {
            CHECK_EQ(result[i], doctest::Approx(expect[i]).epsilon(0.0001));
        }
    }
}

/** @} */
