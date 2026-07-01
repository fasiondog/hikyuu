/*
 * test_SUMBARS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-7
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/SUMBARS.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_indicator_SUMBARS test_indicator_SUMBARS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SUMBARS") {
    Indicator result;

    PriceList a;

    for (int i = 0; i < 10; i++) {
        a.push_back(i * 10);
    }
    Indicator data = PRICELIST(a);

    result = SUMBARS(data, 10);
    CHECK_EQ(result.name(), "SUMBARS");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 15);
    CHECK_EQ(result.name(), "SUMBARS");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 90);
    CHECK_EQ(result.name(), "SUMBARS");
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 4);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
    CHECK_UNARY(std::isnan(result[2]));
    CHECK_UNARY(std::isnan(result[3]));
    CHECK_EQ(result[4], 2);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 1);
    CHECK_EQ(result[7], 1);
    CHECK_EQ(result[8], 1);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 0);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result[1], 0);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);

    result = SUMBARS(data, 170);
    CHECK_EQ(result.discard(), 6);
    CHECK_EQ(result.size(), 10);
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[6], 3);
    CHECK_EQ(result[7], 2);
    CHECK_EQ(result[8], 2);
    CHECK_EQ(result[9], 1);

    result = SUMBARS(data, 450);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 9);
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    CHECK_EQ(result[9], 8);

    result = SUMBARS(data, 451);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 10);
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    result = SUMBARS(CVAL(10), 451);
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.size(), 1);
    CHECK_UNARY(std::isnan(result[0]));

    result = SUMBARS(CVAL(10), 10);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    a.clear();
    a.push_back(10);
    a.push_back(20);
    data = PRICELIST(a);
    result = SUMBARS(data, 20);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 0);

    result = SUMBARS(data, 0);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 0);

    result = SUMBARS(data, 30);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], 1);

    result = SUMBARS(data, 40);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 2);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_UNARY(std::isnan(result[1]));
}

/** @par 检测点 */
TEST_CASE("test_SUMBARS_dyn") {
    Stock stock = StockManager::instance().getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-30));
    // KData kdata = stock.getKData(KQuery(0, Null<size_t>(), KQuery::MIN));
    Indicator c = CLOSE(kdata);
    Indicator expect = SUMBARS(c, 10);
    Indicator result = SUMBARS(c, CVAL(c, 10));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    result = SUMBARS(c, IndParam(CVAL(c, 10)));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }

    expect = SUMBARS(c, 0);
    result = SUMBARS(c, CVAL(c, 0));
    CHECK_EQ(expect.size(), result.size());
    // CHECK_EQ(expect.discard(), result.discard());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        CHECK_EQ(expect[i], doctest::Approx(result[i]));
    }
}

//-----------------------------------------------------------------------------
// 动态/静态路径符号回归（修复 _dyn_calculate 返回负号的 bug）
//-----------------------------------------------------------------------------
/** @par 检测点 */
TEST_CASE("test_SUMBARS_dyn_sign_regression") {
    // 反例：末位需向左回溯 2 根才累加到 a，距离 > 0（非退化解）
    //   修复前动态版返回 -2（符号错误），修复后返回 2（与静态版一致）
    PriceList a;
    a.push_back(5);
    a.push_back(3);
    a.push_back(2);  // [5, 3, 2]
    Indicator data = PRICELIST(a);

    // 静态路径（标量参数）
    Indicator s = SUMBARS(data, 10);
    CHECK_UNARY(std::isnan(s[0]));
    CHECK_UNARY(std::isnan(s[1]));
    CHECK_EQ(s[2], 2);  // i=2 向左回溯到 j=0, 距离 2

    // 动态路径（IndParam 序列参数，常量序列）
    Indicator d1 = SUMBARS(data, CVAL(data, 10));
    CHECK_UNARY(std::isnan(d1[0]));
    CHECK_UNARY(std::isnan(d1[1]));
    CHECK_EQ(d1[2], 2);  // 修复前为 -2

    Indicator d2 = SUMBARS(data, IndParam(CVAL(data, 10)));
    CHECK_EQ(d2[2], 2);

    // 动态/静态逐位数值对称（discard 不要求相等，见下用例）
    CHECK_EQ(s.size(), d1.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (std::isnan(s[i])) {
            CHECK_UNARY(std::isnan(d1[i]));
        } else {
            CHECK_EQ(s[i], doctest::Approx(d1[i]));
        }
    }
}

/** @par 检测点 */
TEST_CASE("test_SUMBARS_dyn_static_equivalence") {
    // 多个位置的距离 > 0，覆盖非退化解等价类
    PriceList a;
    for (int i = 0; i < 10; i++) {
        a.push_back(i * 10);  // [0,10,20,30,40,50,60,70,80,90]
    }
    Indicator data = PRICELIST(a);

    // 静态: discard=4, [4]=2, [5..8]=1, [9]=0
    Indicator s = SUMBARS(data, 90);
    CHECK_EQ(s[4], 2);
    CHECK_EQ(s[5], 1);
    CHECK_EQ(s[6], 1);
    CHECK_EQ(s[7], 1);
    CHECK_EQ(s[8], 1);
    CHECK_EQ(s[9], 0);

    auto check_eq = [](const Indicator& x, const Indicator& y) {
        CHECK_EQ(x.size(), y.size());
        for (size_t i = 0; i < x.size(); i++) {
            if (std::isnan(x[i])) {
                CHECK_UNARY(std::isnan(y[i]));
            } else {
                CHECK_EQ(x[i], doctest::Approx(y[i]));
            }
        }
    };

    check_eq(s, SUMBARS(data, CVAL(data, 90)));
    check_eq(s, SUMBARS(data, IndParam(CVAL(data, 90))));
}

/** @par 检测点 */
TEST_CASE("test_SUMBARS_dyn_unreachable_discard") {
    // 全段累加不可达：动态版仅逐位写 NaN 且不推进 discard，静态版整体丢弃
    PriceList a;
    for (int i = 0; i < 10; i++) {
        a.push_back(1);  // 全 1
    }
    Indicator data = PRICELIST(a);

    Indicator s = SUMBARS(data, 100);  // 全段不可达
    CHECK_EQ(s.discard(), s.size());  // 静态版整体丢弃
    for (size_t i = 0; i < s.size(); i++) {
        CHECK_UNARY(std::isnan(s[i]));
    }

    Indicator d = SUMBARS(data, CVAL(data, 100));
    CHECK_EQ(d.discard(), data.discard());  // 动态版不扩大 discard（== 输入 discard）
    for (size_t i = 0; i < d.size(); i++) {
        CHECK_UNARY(std::isnan(d[i]));
    }
}

/** @par 检测点 */
TEST_CASE("test_SUMBARS_dyn_edge") {
    // 1. a <= 0：首根即 sum>=a 成立，距离恒为 0（验证 i==j 退化解，不死循环）
    {
        PriceList a;
        for (int i = 0; i < 5; i++) a.push_back(i + 1);  // [1,2,3,4,5]
        Indicator data = PRICELIST(a);

        Indicator s0 = SUMBARS(data, 0);
        Indicator d0 = SUMBARS(data, CVAL(data, 0));
        for (size_t i = 0; i < data.size(); i++) {
            CHECK_EQ(s0[i], 0);
            CHECK_EQ(d0[i], 0);
        }

        Indicator sneg = SUMBARS(data, -5);
        Indicator dneg = SUMBARS(data, CVAL(data, -5));
        for (size_t i = 0; i < data.size(); i++) {
            CHECK_EQ(sneg[i], 0);
            CHECK_EQ(dneg[i], 0);
        }
    }

    // 2. 输入序列带 discard > 0：内层 j 循环下界为 ind.discard()，验证终止不越界
    {
        PriceList a;
        a.push_back(10);
        for (int i = 0; i < 6; i++) a.push_back(1);  // [10,1,1,1,1,1,1]
        Indicator ma = MA(PRICELIST(a), 2);  // discard=1, [1]=(10+1)/2=5.5, 之后恒为 1
        CHECK_EQ(ma.discard(), 1);

        Indicator s = SUMBARS(ma, 2);
        Indicator d = SUMBARS(ma, CVAL(ma, 2));
        CHECK_EQ(d.discard(), ma.discard());
        CHECK_EQ(s[1], 0);  // j=1 sum=5.5>=2
        CHECK_EQ(d[1], 0);
        CHECK_EQ(s[2], 1);  // j=2 sum=1; j=1 sum=6.5>=2 -> i-j=1
        CHECK_EQ(d[2], 1);
        CHECK_EQ(s[3], 1);
        CHECK_EQ(d[3], 1);
    }

    // 3. 极端长度
    {
        // 空序列：不崩溃
        PriceList empty;
        Indicator data = PRICELIST(empty);
        Indicator s = SUMBARS(data, 10);
        Indicator d = SUMBARS(data, CVAL(data, 10));
        CHECK_EQ(s.size(), 0);
        CHECK_EQ(d.size(), 0);

        // 单根可达（静态路径可用 CVAL；动态路径用 PRICELIST 构造 size=1 的 a 序列，
        // 规避 CVAL 嵌套在单元素上 size 传播为 0 的缺陷）
        Indicator one = CVAL(10);
        CHECK_EQ(SUMBARS(one, 10)[0], 0);
        PriceList one_pl;
        one_pl.push_back(10);
        CHECK_EQ(SUMBARS(one, IndParam(PRICELIST(one_pl)))[0], 0);

        // 单根不可达
        CHECK_UNARY(std::isnan(SUMBARS(one, 20)[0]));
        PriceList unreach_pl;
        unreach_pl.push_back(20);
        CHECK_UNARY(std::isnan(SUMBARS(one, IndParam(PRICELIST(unreach_pl)))[0]));
    }
}

/** @par 检测点 */
TEST_CASE("test_SUMBARS_dyn_varying_param") {
    // 动态参数 a 序列逐位变化，手工核算
    PriceList a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    a.push_back(2);
    a.push_back(1);  // data=[1,2,3,2,1]
    Indicator data = PRICELIST(a);

    PriceList pa;
    pa.push_back(5);
    pa.push_back(100);
    pa.push_back(3);
    pa.push_back(3);
    pa.push_back(3);  // a=[5,100,3,3,3]
    Indicator aseq = PRICELIST(pa);

    Indicator d = SUMBARS(data, IndParam(aseq));
    CHECK_UNARY(std::isnan(d[0]));  // a=5, 累加至 j=0 sum=1 <5
    CHECK_UNARY(std::isnan(d[1]));  // a=100, 不可达
    CHECK_EQ(d[2], 0);              // a=3, j=2 sum=3>=3
    CHECK_EQ(d[3], 1);             // a=3, j=3 sum=2, j=2 sum=5>=3 -> i-j=1
    CHECK_EQ(d[4], 1);              // a=3, j=4 sum=1, j=3 sum=3>=3 -> i-j=1
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SUMBARS_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SUMBARS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SUMBARS(CLOSE(kdata), 10000);
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

    CHECK_EQ(x2.name(), "SUMBARS");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
