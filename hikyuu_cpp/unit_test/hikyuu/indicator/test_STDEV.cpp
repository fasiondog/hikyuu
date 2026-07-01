/*
 * test_STD.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/STDEV.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_STDEV test_indicator_STDEV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_STDEV") {
    /** @arg n > 1 的正常情况 */
    PriceList d;
    for (size_t i = 0; i < 15; ++i) {
        d.push_back(i + 1);
    }
    d[5] = 4.0;
    d[7] = 4.0;
    d[11] = 6.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 10);
    // BREAKING CHANGE: discard 从 1 改为 data.discard()+n-1=9（窗口未满输出 NaN）
    CHECK_EQ(dev.discard(), 9);
    CHECK_EQ(dev.size(), 15);

    // 窗口未满（i<9）输出 NaN；i>=9 稳态值与旧实现一致（Welford 精度内）
    vector<price_t> expected{0, 0, 0, 0, 0, 0, 0, 0, 0, 2.92309, 3.14289, 2.83039,
                             3.26769, 3.653, 4.00139};
    for (size_t i = 0; i < dev.discard(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
    for (size_t i = dev.discard(); i < dev.size(); i++) {
        CHECK_EQ(dev[i], doctest::Approx(expected[i]).epsilon(0.0001));
    }

    /** @arg n = 1时 */
    CHECK_THROWS_AS(STDEV(ind, 1), std::exception);

    /** @arg operator() */
    Indicator expect = STDEV(ind, 10);
    dev = STDEV(10);
    Indicator result = dev(ind);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = expect.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

/** @par 检测点 */
TEST_CASE("test_STDEV_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = STDEV(c, 10);
    Indicator result = STDEV(c, CVAL(c, 10));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = STDEV(c, IndParam(CVAL(c, 10)));
    // CHECK_EQ(expect.discard(), result.discard());
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_STDEV_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/STDEV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = STDEV(CLOSE(kdata), 10);
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

    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

//----------------------------------------------------------------------------
// NaN 语义测试（Welford 滚动方差 + 离群值离开条件重算）
//----------------------------------------------------------------------------

/** @par 检测点：散布 NaN 的滚动标准差 */
TEST_CASE("test_STDEV_nan_scattered") {
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
    Indicator dev = STDEV(ind, 4);
    CHECK_EQ(dev.discard(), 3);
    for (size_t i = 0; i < dev.discard(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
    // i=3 窗口 [1,2,3,NaN] → vc=3, std=std(1,2,3)=1.0
    CHECK_EQ(dev[3], doctest::Approx(1.0).epsilon(0.0001));
    // i=4 窗口 [2,3,NaN,5] → vc=3, std=std(2,3,5)=1.527525
    CHECK_EQ(dev[4], doctest::Approx(1.527525).epsilon(0.0001));
    // i=5 窗口 [3,NaN,5,6] → vc=3, std=std(3,5,6)=1.527525
    CHECK_EQ(dev[5], doctest::Approx(1.527525).epsilon(0.0001));
    // i=6 窗口 [NaN,5,6,7] → vc=3, std=std(5,6,7)=1.0（恢复）
    CHECK_EQ(dev[6], doctest::Approx(1.0).epsilon(0.0001));
    // i=7 窗口 [5,6,7,8] → vc=4, std=std(5,6,7,8)=1.290994
    CHECK_EQ(dev[7], doctest::Approx(1.290994).epsilon(0.0001));
    CHECK_EQ(dev[8], doctest::Approx(1.290994).epsilon(0.0001));
    CHECK_EQ(dev[9], doctest::Approx(1.290994).epsilon(0.0001));
}

/** @par 检测点：连续 NaN 穿越后状态恢复 */
TEST_CASE("test_STDEV_nan_consecutive") {
    PriceList d;
    d.push_back(1.0);
    d.push_back(2.0);
    d.push_back(Null<price_t>());
    d.push_back(Null<price_t>());
    d.push_back(Null<price_t>());
    d.push_back(3.0);
    d.push_back(4.0);

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 3);
    CHECK_EQ(dev.discard(), 2);
    // i=2 窗口 [1,2,NaN] → vc=2, std=std(1,2)=0.707107
    CHECK_EQ(dev[2], doctest::Approx(0.707107).epsilon(0.0001));
    // i=3 窗口 [2,NaN,NaN] → vc=1, std=NaN
    CHECK_UNARY(std::isnan(dev[3]));
    // i=4 窗口 [NaN,NaN,NaN] → vc=0, std=NaN
    CHECK_UNARY(std::isnan(dev[4]));
    // i=5 窗口 [NaN,NaN,3] → vc=1, std=NaN（从 0 重建）
    CHECK_UNARY(std::isnan(dev[5]));
    // i=6 窗口 [NaN,3,4] → vc=2, std=std(3,4)=0.707107（恢复）
    CHECK_EQ(dev[6], doctest::Approx(0.707107).epsilon(0.0001));
}

/** @par 检测点：窗口仅 1 个有效值（count=1，std=NaN 而非 Inf/异常） */
TEST_CASE("test_STDEV_nan_single_valid") {
    PriceList d;
    for (int i = 0; i < 7; ++i) {
        d.push_back(Null<price_t>());
    }
    d[3] = 5.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 4);
    CHECK_EQ(dev.discard(), 3);
    // vc 恒=1，std 恒 NaN（样本方差除以 0，安全返回 NaN）
    for (size_t i = dev.discard(); i < dev.size(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
}

/** @par 检测点：大基数小方差（Welford 精度验证） */
TEST_CASE("test_STDEV_large_base") {
    PriceList d;
    double base = 1e8;
    for (int i = 1; i <= 5; ++i) {
        d.push_back(base + i);
    }

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 3);
    CHECK_EQ(dev.discard(), 2);
    // 窗口 [1e8+1,1e8+2,1e8+3] → std=1.0（精确，Welford 消除大基数）
    CHECK_EQ(dev[2], doctest::Approx(1.0).epsilon(0.0001));
    CHECK_EQ(dev[3], doctest::Approx(1.0).epsilon(0.0001));
    CHECK_EQ(dev[4], doctest::Approx(1.0).epsilon(0.0001));
}

/** @par 检测点：离群值离开窗口触发重算（精度恢复） */
TEST_CASE("test_STDEV_outlier_leaving_rescan") {
    PriceList d;
    d.push_back(10.0);
    d.push_back(10.0);
    d.push_back(10.0);
    d.push_back(1e6);  // 离群值
    d.push_back(2.0);
    d.push_back(4.0);
    d.push_back(4.0);
    d.push_back(4.0);
    d.push_back(5.0);
    d.push_back(5.0);

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 5);
    CHECK_EQ(dev.discard(), 4);
    // i=8 离群值 1e6 离开窗口，窗口 [2,4,4,4,5]
    // 真实 std = sqrt(1.2) ≈ 1.095445，重算必须精确恢复
    CHECK_EQ(dev[8], doctest::Approx(1.095445).epsilon(0.00001));
    // i=9 窗口 [4,4,4,5,5] → std=0.547723
    CHECK_EQ(dev[9], doctest::Approx(0.547723).epsilon(0.0001));
}

/** @par 检测点：极小窗口 n=2 */
TEST_CASE("test_STDEV_n2") {
    PriceList d;
    d.push_back(3.0);
    d.push_back(Null<price_t>());
    d.push_back(5.0);
    d.push_back(7.0);

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 2);
    CHECK_EQ(dev.discard(), 1);
    // i=1 窗口 [3,NaN] → vc=1, std=NaN
    CHECK_UNARY(std::isnan(dev[1]));
    // i=2 窗口 [NaN,5] → vc=1, std=NaN
    CHECK_UNARY(std::isnan(dev[2]));
    // i=3 窗口 [5,7] → vc=2, std=std(5,7)=1.414214
    CHECK_EQ(dev[3], doctest::Approx(1.414214).epsilon(0.0001));
}

/** @par 检测点：全 NaN 序列不死循环不抛异常 */
TEST_CASE("test_STDEV_all_nan") {
    PriceList d;
    for (int i = 0; i < 4; ++i) {
        d.push_back(Null<price_t>());
    }
    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 3);
    CHECK_EQ(dev.size(), 4);
    for (size_t i = 0; i < dev.size(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
}

/** @par 检测点：上游 discard 传递 */
TEST_CASE("test_STDEV_upstream_discard") {
    PriceList d;
    for (int i = 0; i < 10; ++i) {
        d.push_back(i + 1);
    }
    for (int i = 0; i < 4; ++i) {
        d[i] = Null<price_t>();
    }
    Indicator ind = PRICELIST(d);
    ind.setDiscard(4);

    Indicator dev = STDEV(ind, 3);
    // m_discard = 4 + 3 - 1 = 6
    CHECK_EQ(dev.discard(), 6);
    for (size_t i = 0; i < dev.discard(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }
    // i=6 窗口 [5,6,7] → std=1.0
    CHECK_EQ(dev[6], doctest::Approx(1.0).epsilon(0.0001));
}

/** @par 检测点：_dyn 动态路径与静态 n 等价（含 NaN） */
TEST_CASE("test_STDEV_dyn_nan_equivalence") {
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
    Indicator expect = STDEV(ind, 4);
    Indicator result = STDEV(ind, CVAL(ind, 4));
    CHECK_EQ(expect.size(), result.size());
    // _dyn 路径的 discard 由 updateDiscard 自动检测，与静态 m_discard 可能不同
    // （既有设计，见 test_STDEV_dyn 中注释掉的 discard 检查）
    // 只验证两者 common discard 之后的位置值一致
    size_t common_discard = std::max(expect.discard(), result.discard());
    for (size_t i = common_discard; i < result.size(); ++i) {
        if (std::isnan(expect[i])) {
            CHECK_UNARY(std::isnan(result[i]));
        } else {
            CHECK_EQ(result[i], doctest::Approx(expect[i]).epsilon(0.0001));
        }
    }
}

/** @par 检测点：n=1 参数被拦截（样本标准差无定义，抛异常而非 Inf/崩溃） */
TEST_CASE("test_STDEV_n1_rejected") {
    PriceList d;
    for (int i = 0; i < 5; ++i) {
        d.push_back(i + 1);
    }
    Indicator ind = PRICELIST(d);
    // n=1: 样本方差除以 N-1=0，_checkParam 拦截（HKU_ASSERT(n==0 || n>=2)）
    CHECK_THROWS_AS(STDEV(ind, 1), std::exception);
}

/** @par 检测点：常数序列 Zero Variance（浮点噪声防御） */
TEST_CASE("test_STDEV_zero_variance") {
    PriceList d;
    for (int i = 0; i < 5; ++i) {
        d.push_back(3.14);
    }
    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 3);
    CHECK_EQ(dev.discard(), 2);
    // 常数序列方差=0，浮点噪声可能让 M2 微负，max(0,...) 防御确保输出 0.0 而非 NaN
    for (size_t i = dev.discard(); i < dev.size(); ++i) {
        CHECK_EQ(dev[i], doctest::Approx(0.0).epsilon(0.0001));
    }
}

/** @} */
