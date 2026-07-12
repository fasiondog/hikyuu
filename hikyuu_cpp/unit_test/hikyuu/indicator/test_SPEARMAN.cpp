/*
 * test_COS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/SPEARMAN.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_COS test_indicator_CORR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

static void spearmanLevel(const IndicatorImp::value_t *data, IndicatorImp::value_t *level,
                          size_t total) {
    std::vector<std::pair<IndicatorImp::value_t, size_t>> data_index(total);
    for (size_t i = 0; i < total; i++) {
        data_index[i].first = data[i];
        data_index[i].second = i;
    }

    std::sort(data_index.begin(), data_index.end(),
              [](const std::pair<IndicatorImp::value_t, size_t> &a,
                 const std::pair<IndicatorImp::value_t, size_t> &b) {
                  if (std::isnan(a.first)) {
                      return false;
                  }
                  if (std::isnan(b.first) && !std::isnan(a.first)) {
                      return true;
                  }
                  return a.first < b.first;
              });

    IndicatorImp::value_t null_value = Null<IndicatorImp::value_t>();
    int64_t pos = (int64_t)total - 1;
    while (pos > 0) {
        if (!std::isnan(data_index[pos].first)) {
            break;
        }
        level[data_index[pos].second] = null_value;
        pos--;
    }

    int64_t len = pos + 1;
    int64_t i = 0;
    while (i < len) {
        size_t count = 1;
        IndicatorImp::value_t score = i + 1.0;
        for (int64_t j = i + 1; j < len; j++) {
            if (data_index[i].first != data_index[j].first) {
                break;
            }
            count++;
            score += j + 1;
        }
        score = score / count;
        for (size_t j = 0; j < count; j++) {
            level[data_index[i + j].second] = score;
        }
        i += count;
    }
}

/** @par 检测点 */
TEST_CASE("test_spearmanLevel") {
    /** @arg 无重复值排序 */
    std::vector<IndicatorImp::value_t> a{3., 8., 4., 7., 2.};
    size_t totala = a.size();
    auto levela = std::make_unique<IndicatorImp::value_t[]>(totala);
    auto *ptra = levela.get();
    spearmanLevel(a.data(), levela.get(), totala);

    std::vector<IndicatorImp::value_t> expecta = {2., 5., 3., 4., 1.};
    for (size_t i = 0; i < totala; i++) {
        CHECK_EQ(ptra[i], doctest::Approx(expecta[i]));
    }

    /** @arg 存在重复值 */
    std::vector<IndicatorImp::value_t> b{5., 10., 8., 10., 6.};
    size_t totalb = b.size();
    auto levelb = std::make_unique<IndicatorImp::value_t[]>(totalb);
    auto *ptrb = levelb.get();
    spearmanLevel(b.data(), levelb.get(), totalb);

    std::vector<IndicatorImp::value_t> expectb = {1., 4.5, 3., 4.5, 2.};
    for (size_t i = 0; i < totalb; i++) {
        CHECK_EQ(ptrb[i], doctest::Approx(expectb[i]));
    }

    /** @arg 存在nan值*/
    IndicatorImp::value_t null_value = Null<IndicatorImp::value_t>();
    std::vector<IndicatorImp::value_t> c{
      null_value, null_value, 5., 10., null_value, null_value, 8., 10., 6., null_value, null_value};
    size_t totalc = c.size();
    auto levelc = std::make_unique<IndicatorImp::value_t[]>(totalc);
    auto *ptrc = levelc.get();
    spearmanLevel(c.data(), levelc.get(), totalc);

    std::vector<IndicatorImp::value_t> expectc = {null_value, 1.,  4.5, null_value,
                                                  3.,         4.5, 2.,  null_value};
    CHECK_UNARY(std::isnan(ptrc[0]));
    CHECK_UNARY(std::isnan(ptrc[1]));
    CHECK_EQ(ptrc[2], doctest::Approx(1.));
    CHECK_EQ(ptrc[3], doctest::Approx(4.5));
    CHECK_UNARY(std::isnan(ptrc[4]));
    CHECK_UNARY(std::isnan(ptrc[5]));
    CHECK_EQ(ptrc[6], doctest::Approx(3.));
    CHECK_EQ(ptrc[7], doctest::Approx(4.5));
    CHECK_EQ(ptrc[8], doctest::Approx(2.));
    CHECK_UNARY(std::isnan(ptrc[9]));
    CHECK_UNARY(std::isnan(ptrc[10]));
}

/** @par 检测点 */
TEST_CASE("test_SPEARMAN") {
    Indicator result;

    /** @arg 空指标 */
    result = SPEARMAN(Indicator(), Indicator(), 10);
    CHECK_UNARY(result.empty());

    PriceList a{3., 8., 4., 7., 2.};
    PriceList b{5., 10., 8., 10., 6.};

    Indicator x = PRICELIST(a);
    Indicator y = PRICELIST(b);

    /** @arg 非法参数 n */
    CHECK_THROWS_AS(SPEARMAN(x, y, -1), std::exception);
    CHECK_THROWS_AS(SPEARMAN(x, y, 1), std::exception);

    /** @arg 正常情况 n */
    PriceList expect{Null<price_t>(), 1., 1., 0.95, 0.872082};
    result = SPEARMAN(x, y, a.size());
    CHECK_EQ(result.name(), "SPEARMAN");
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result.size(), a.size());
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]));
    }

    expect = {Null<price_t>(), Null<price_t>(), 1., 0.866025, 1.};
    result = SPEARMAN(x, y, 3);
    CHECK_EQ(result.name(), "SPEARMAN");
    CHECK_EQ(result.discard(), 2);
    CHECK_EQ(result.size(), a.size());
    for (size_t i = result.discard(); i < result.size(); i++) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]));
    }

    /** @arg 包含 nan 值 */
    price_t null_value = Null<price_t>();
    a = PriceList{3., 8., null_value, 4., 7., 2., null_value, null_value};
    b = PriceList{null_value, 5., 10., 8., null_value, 10., 6., null_value};
    x = PRICELIST(a);
    y = PRICELIST(b);
    result = SPEARMAN(x, y, 4);
    CHECK_EQ(result.name(), "SPEARMAN");
    CHECK_EQ(result.discard(), 3);
    CHECK_EQ(result.size(), x.size());
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[5], doctest::Approx(-1.));
    CHECK_EQ(result[6], doctest::Approx(-1.));
    CHECK_UNARY(std::isnan(result[7]));
}

/** @par 检测点 */
TEST_CASE("test_SPEARMAN_n0_full_window") {
    // 回归 n=0 默认参数：_calculate 将 n 归一化为 total 后委托
    // _increment_calculate，后者必须再次归一化，否则窗口长度为 0、全 NaN。

    /** @arg n=0 等价于 n=total 全窗口 */
    PriceList a{3., 8., 4., 7., 2.};
    PriceList b{5., 10., 8., 10., 6.};
    Indicator x = PRICELIST(a);
    Indicator y = PRICELIST(b);
    Indicator result = SPEARMAN(x, y, 0);
    CHECK_EQ(result.name(), "SPEARMAN");
    CHECK_EQ(result.size(), a.size());
    CHECK_EQ(result.discard(), 4);
    // 硬编码期望钉死修复前“全 NaN”退路；并与显式 n=total 对照
    CHECK_EQ(result[4], doctest::Approx(0.872082));
    Indicator full = SPEARMAN(x, y, static_cast<int>(a.size()));
    CHECK_EQ(result.discard(), full.discard());
    CHECK_EQ(result[4], doctest::Approx(full[4]));

    /** @arg 空序列 n=0：应安全返回空指标，不越界 */
    result = SPEARMAN(Indicator(), Indicator(), 0);
    CHECK_UNARY(result.empty());

    /** @arg 极短序列 total=1, n=0：归一化 n=1，m_discard=0+1-1=0；
     *  act_count=1<2 不写值，result[0] 为 NaN（discard 仍为 0，非全量） */
    PriceList short_a{1.};
    PriceList short_b{2.};
    result = SPEARMAN(PRICELIST(short_a), PRICELIST(short_b), 0);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_UNARY(std::isnan(result[0]));

    /** @arg 下界触发 total=2, n=0：归一化 n=2，恰好 act_count==2，完全负相关 */
    PriceList edge_a{1., 2.};
    PriceList edge_b{2., 1.};
    result = SPEARMAN(PRICELIST(edge_a), PRICELIST(edge_b), 0);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result[1], doctest::Approx(-1.0));

    /** @arg 输入带 discard：有效数据不足以填满全窗口时 discard=total */
    // total=5, ind.discard=2 → m_discard = 2 + 5 - 1 = 6 > 5 → discard=5
    PriceList da{1., 2., 3., 4., 5.};
    PriceList db{5., 4., 3., 2., 1.};
    result = SPEARMAN(PRICELIST(da, 2), PRICELIST(db, 2), 0);
    CHECK_EQ(result.size(), 5);
    CHECK_EQ(result.discard(), 5);
}

/** @par 检测点 */
TEST_CASE("test_SPEARMAN_with_ties") {
    // 回归 tie-handling: 当输入存在相同值(average-rank)时, 简化公式
    // 1 - 6*sum_d2/(n^3-n) 不再精确, 必须用 Pearson-on-ranks 计算。
    // 下列期望值由 tie 修正公式独立推导, 不依赖本实现:
    //   rho = [n(n^2-1)/6 - T_x - T_y - sum_d2]
    //         / [2*sqrt((n(n^2-1)/12 - T_x)(n(n^2-1)/12 - T_y))]

    /** @arg 双向 tie: a 与 b 均含重复值 (T_x = T_y = 1.0)
     *  a={3,3,5,7,7} -> ranks {1.5,1.5,3,4.5,4.5}
     *  b={10,8,8,6,10} -> ranks {4.5,2.5,2.5,1,4.5}
     *  buggy 简化公式给出 -0.125, 正确值 -0.25 */
    PriceList a{3., 3., 5., 7., 7.};
    PriceList b{10., 8., 8., 6., 10.};
    Indicator x = PRICELIST(a);
    Indicator y = PRICELIST(b);
    Indicator result = SPEARMAN(x, y, a.size());
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result[4], doctest::Approx(-0.25));

    /** @arg 零方差退化: a 全相同 (rank 全为 3), 因子无区分度, 应返回 0.0
     *  buggy 简化公式给出 0.5 (把"无区分度"误判为正相关) */
    PriceList za{5., 5., 5., 5., 5.};
    PriceList zb{1., 2., 3., 4., 5.};
    result = SPEARMAN(PRICELIST(za), PRICELIST(zb), za.size());
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result[4], doctest::Approx(0.0));

    /** @arg 双向 tie + NaN 过滤 (act_count < n): 验证去除 act_count==n 分支后
     *  仅对有效对计算仍正确。窗口 n=7, 有效对 4 个:
     *  (3,10),(5,8),(7,6),(7,10) -> ranks a{1,2,3.5,3.5} b{3.5,2,1,3.5}
     *  正确值 -0.388889 */
    price_t null_value = Null<price_t>();
    PriceList na{3., 3., null_value, 5., 7., 7., null_value};
    PriceList nb{10., null_value, 8., 8., 6., 10., null_value};
    result = SPEARMAN(PRICELIST(na), PRICELIST(nb), 7);
    CHECK_EQ(result[6], doctest::Approx(-0.388889));
}

//-----------------------------------------------------------------------------
// benchmark
//-----------------------------------------------------------------------------
#if ENABLE_BENCHMARK_TEST
TEST_CASE("test_SPEARMAN_benchmark") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    Indicator c = kdata.close();
    Indicator h = kdata.close();
    int cycle = 10;  // 测试循环次数

    {
        BENCHMARK_TIME_MSG(test_SPEARMAN_benchmark, cycle, fmt::format("data len: {}", c.size()));
        SPEND_TIME_CONTROL(false);
        for (int i = 0; i < cycle; i++) {
            Indicator result = SPEARMAN(c, h, 200);
        }
    }
}
#endif

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SPEARMAN_export") {
    StockManager &sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SPEARMAN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SPEARMAN(CLOSE(kdata), OPEN(kdata), 10);
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

    CHECK_EQ(x2.name(), "SPEARMAN");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]));
        }
    }

    Indicator x11 = x1.getResult(1);
    Indicator x21 = x2.getResult(1);
    CHECK_EQ(x11.size(), x21.size());
    CHECK_EQ(x11.discard(), x21.discard());
    CHECK_EQ(x11.getResultNumber(), x21.getResultNumber());
    for (size_t i = x11.discard(); i < x21.size(); ++i) {
        if (std::isnan(x11[i])) {
            CHECK_UNARY(std::isnan(x21[i]));
        } else {
            CHECK_EQ(x11[i], doctest::Approx(x21[i]));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
