/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-26
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <ta-lib/ta_libc.h>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator_talib/ta_crt.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/ALIGN.h>

using namespace hku;

/**
 * @defgroup test_indicator_TA_MAVP test_indicator_TA_MAVP
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

static Indicator expect_output(const Indicator& in1, const Indicator& in2, int min_n = 2,
                               int max_n = 30, int matype = 0) {
    HKU_CHECK(in1.size() == in2.size(), "in1 size: {}, in2 size: {}", in1.size(), in2.size());
    size_t total = in1.size();
    PriceList buf(total, Null<double>());
    const auto* src0 = in1.data();
    const auto* src1 = in2.data();
    int lookback = TA_MAVP_Lookback(min_n, max_n, (TA_MAType)matype);
    int discard = lookback + std::max(in1.discard(), in2.discard());
    int outBegIdx;
    int outNbElement;
    TA_MAVP(discard, total - 1, src0, src1, min_n, max_n, (TA_MAType)matype, &outBegIdx,
            &outNbElement, buf.data() + discard);
    return PRICELIST(buf, discard);
}

static void check_output(const Indicator& result, const Indicator& in1, const Indicator& in2) {
    Indicator expect = expect_output(in1, in2);
    for (size_t i = expect.discard(); i < expect.size(); i++) {
        HKU_INFO("{}: {}", i, expect[i]);
    }
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result.get(i)));
    }
    for (size_t i = result.discard(), total = result.size(); i < total; i++) {
        // HKU_INFO("{}: {}", i, result[i]);
        CHECK_EQ(result.get(i), expect.get(i));
    }
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.discard(), expect.discard());
}

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_params") {
    KData k1 = getKData("sz000001", KQuery(-35));
    KData k2 = getKData("sz000002", KQuery(-35));
    Indicator result, a, b, expect;

    /** @arg 非法参数 */
    CHECK_THROWS(TA_MAVP(CVAL(1), 1));
    CHECK_THROWS(TA_MAVP(CVAL(1), 100001));
    CHECK_THROWS(TA_MAVP(CVAL(1), 2, 1));
    CHECK_THROWS(TA_MAVP(CVAL(1), 2, 100001));

    /** @arg 输入两个空指标, 不指定上下文 */
    result = TA_MAVP(Indicator(), Indicator());
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.name(), "TA_MAVP(Indicator)");

    /** @arg 输入两个空指标, 指定上下文 */
    result = TA_MAVP(Indicator(), Indicator())(k1);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.name(), "TA_MAVP(Indicator)");
}

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_all_not_time_without_context") {
    //-------------------------------------------------------
    // 不指定上下文, 输入序列均为时间无关序列，计算a, 参考b
    //-------------------------------------------------------
    Indicator result, a, b, expect;
    double nan = Null<double>();

    /** @arg a,b 长度都为1 (长度相等且小于2)，不指定上下文 */
    a = PRICELIST(PriceList{1.});
    b = PRICELIST(PriceList{2.});
    CHECK_EQ(a.size(), 1);
    CHECK_EQ(b.size(), 1);
    result = TA_MAVP(a, b);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.name(), "TA_MAVP(PRICELIST)");
    CHECK_UNARY(std::isnan(result[0]));

    /** @arg a 长度为0，b 长度为1，不指定上下文 */
    a = PRICELIST();
    b = PRICELIST(PriceList{1.});
    CHECK_EQ(a.size(), 0);
    CHECK_EQ(b.size(), 1);
    result = TA_MAVP(a, b);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg a 长度为2，b 长度为2，不指定上下文 */
    a = PRICELIST(PriceList{1., 2.});
    b = PRICELIST(PriceList{1., 2.});
    CHECK_EQ(a.size(), 2);
    CHECK_EQ(b.size(), 2);
    result = TA_MAVP(a, b);
    check_output(result, a, b);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 2);

    /** @arg a 长度为29，b 长度为20，不指定上下文 */
    a = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29});
    b = PRICELIST(PriceList{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20});
    CHECK_EQ(a.size(), 29);
    CHECK_EQ(b.size(), 20);
    result = TA_MAVP(a, b);
    check_output(
      result, a,
      PRICELIST(PriceList{nan, nan, nan, nan, nan, nan, nan, nan, nan, 1,  2,  3,  4,  5, 6,
                          7,   8,   9,   10,  11,  12,  13,  14,  15,  16, 17, 18, 19, 20},
                9));
    CHECK_EQ(result.size(), 29);
    CHECK_EQ(result.discard(), 29);

    /** @arg a 长度为20，b 长度为29，不指定上下文 */
    a = PRICELIST(PriceList{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20});
    b = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29});
    CHECK_EQ(a.size(), 20);
    CHECK_EQ(b.size(), 29);
    result = TA_MAVP(a, b);
    check_output(result, a, PRICELIST(PriceList{10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                                                20, 21, 22, 23, 24, 25, 26, 27, 28, 29}));
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.discard(), 20);

    /** @arg a 长度为20，b 长度为29，不指定上下文 */
    a = PRICELIST(PriceList{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20});
    b = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29});
    CHECK_EQ(a.size(), 20);
    CHECK_EQ(b.size(), 29);
    result = TA_MAVP(a, b);
    check_output(result, a, PRICELIST(PriceList{10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                                                20, 21, 22, 23, 24, 25, 26, 27, 28, 29}));
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.discard(), 20);

    /** @arg a 长度为30，b 长度为30，不指定上下文 */
    a = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30});
    b = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30});
    CHECK_EQ(a.size(), 30);
    CHECK_EQ(b.size(), 30);
    result = TA_MAVP(a, b);
    check_output(result, a, b);
    CHECK_EQ(result.size(), 30);
    CHECK_EQ(result.discard(), 29);

    /** @arg a 长度为30，b 长度为35，不指定上下文 */
    a = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30});
    b = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
                            19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35});
    CHECK_EQ(a.size(), 30);
    CHECK_EQ(b.size(), 35);
    result = TA_MAVP(a, b);
    check_output(result, a,
                 PRICELIST(PriceList{6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                                     21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35}));
    CHECK_EQ(result.size(), 30);
    CHECK_EQ(result.discard(), 29);

    /** @arg a 长度为35，b 长度为30，不指定上下文 */
    HKU_INFO("-------------------------");
    a = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
                            19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35});
    b = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30});
    CHECK_EQ(a.size(), 35);
    CHECK_EQ(b.size(), 30);
    result = TA_MAVP(a, b);
    check_output(result, a,
                 PRICELIST(PriceList{nan, nan, nan, nan, nan, 1,  2,  3,  4,  5,  6,  7,
                                     8,   9,   10,  11,  12,  13, 14, 15, 16, 17, 18, 19,
                                     20,  21,  22,  23,  24,  25, 26, 27, 28, 29, 30},
                           5));
    CHECK_EQ(result.size(), 35);
    CHECK_EQ(result.discard(), 34);

    HKU_INFO("result.size(): {}, result.discard(): {}", result.size(), result.discard());

    // for (size_t i = result.discard(), total = result.size(); i < total; i++) {
    //     HKU_INFO("{}: {}", i, result[i]);
    // }
}

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_all_not_time_with_context") {
    //-------------------------------------------------------
    // 指定上下文，输入序列均为时间无关序列，计算a, 参考b
    //-------------------------------------------------------
    KData k1 = getKData("sz000001", KQuery(-35));
    KData k2 = getKData("sz000002", KQuery(-35));
    Indicator result, a, b, expect;
    double nan = Null<double>();

    /** @arg a, b 长度都为1 (长度相等且小于2)，指定上下文 */
    a = PRICELIST(PriceList{1.});
    b = PRICELIST(PriceList{2.});
    CHECK_EQ(a.size(), 1);
    CHECK_EQ(b.size(), 1);
    result = TA_MAVP(a, b)(k1);
    CHECK_EQ(result.size(), 35);
    CHECK_EQ(result.discard(), 35);
    CHECK_EQ(result.name(), "TA_MAVP(PRICELIST)");
    for (size_t i = 0, len = result.discard(); i < len; i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    /** @arg a 长度为0，b 长度为1，指定上下文 */
    a = PRICELIST();
    b = PRICELIST(PriceList{1.});
    CHECK_EQ(a.size(), 0);
    CHECK_EQ(b.size(), 1);
    result = TA_MAVP(a, b)(k1);
    CHECK_EQ(result.size(), 35);
    CHECK_EQ(result.discard(), 35);

    /** @arg a 长度为2，b 长度为2，指定上下文 */
    a = PRICELIST(PriceList{1., 2.});
    b = PRICELIST(PriceList{1., 2.});
    CHECK_EQ(a.size(), 2);
    CHECK_EQ(b.size(), 2);
    result = TA_MAVP(a, b)(k1);
    check_output(result, a(k1), b(k1));
    CHECK_EQ(result.size(), k1.size());
    CHECK_EQ(result.discard(), k1.size());

    /** @arg a 长度为29，b 长度为20，指定上下文 */
    a = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29});
    b = PRICELIST(PriceList{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20});
    CHECK_EQ(a.size(), 29);
    CHECK_EQ(b.size(), 20);
    result = TA_MAVP(a, b)(k1);
    check_output(result,
                 PRICELIST(PriceList{nan, nan, nan, nan, nan, nan, 1,  2,  3,  4,  5,  6,
                                     7,   8,   9,   10,  11,  12,  13, 14, 15, 16, 17, 18,
                                     19,  20,  21,  22,  23,  24,  25, 26, 27, 28, 29},
                           6),
                 PRICELIST(PriceList{nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan,
                                     nan, nan, nan, 1,   2,   3,   4,   5,   6,   7,   8,   9,
                                     10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20},
                           15));
    CHECK_EQ(result.size(), 35);
    CHECK_EQ(result.discard(), 35);

    /** @arg a 长度为20，b 长度为29，指定上下文 */
    a = PRICELIST(PriceList{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20});
    b = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29});
    CHECK_EQ(a.size(), 20);
    CHECK_EQ(b.size(), 29);
    result = TA_MAVP(a, b)(k1);
    check_output(result,
                 PRICELIST(PriceList{nan, nan, nan, nan, nan, nan, 1,  2,  3,  4,  5,  6,
                                     7,   8,   9,   10,  11,  12,  13, 14, 15, 16, 17, 18,
                                     19,  20,  21,  22,  23,  24,  25, 26, 27, 28, 29},
                           6),
                 PRICELIST(PriceList{nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan,
                                     nan, nan, nan, 1,   2,   3,   4,   5,   6,   7,   8,   9,
                                     10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20},
                           15));
    CHECK_EQ(result.size(), 35);
    CHECK_EQ(result.discard(), 35);

    // /** @arg a 长度为20，b 长度为29，不指定上下文 */
    // a = PRICELIST(PriceList{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    // 20}); b = PRICELIST(PriceList{1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    //                         16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29});
    // CHECK_EQ(a.size(), 20);
    // CHECK_EQ(b.size(), 29);
    // result = TA_MAVP(a, b);
    // check_output(result, a, PRICELIST(PriceList{10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    //                                             20, 21, 22, 23, 24, 25, 26, 27, 28, 29}));
    // CHECK_EQ(result.size(), 20);
    // CHECK_EQ(result.discard(), 20);
    // HKU_INFO("result.size(): {}, result.discard(): {}", result.size(), result.discard());

    // for (size_t i = result.discard(), total = result.size(); i < total; i++) {
    //     HKU_INFO("{}: {}", i, result[i]);
    // }
}

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_ref_not_time_ind_is_time_without_context") {
    //-------------------------------------------------------
    // 不指定上下文, 参考指标为时间无关序列，计算指标为时间序列, 计算a, 参考b
    //-------------------------------------------------------
}

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_ref_not_time_ind_is_time_with_context") {
    //-------------------------------------------------------
    // 指定上下文, 参考指标为时间无关序列，计算指标为时间序列, 计算a, 参考b
    //-------------------------------------------------------
}

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_ref_is_prototype_ind_is_time_without_context") {
    //-------------------------------------------------------
    // 不指定上下文, 参考指标为时间序列且为公式原型，计算指标为时间序列, 计算a, 参考b
    //-------------------------------------------------------
}

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_ref_is_prototype_ind_is_time_with_context") {
    //-------------------------------------------------------
    // 指定上下文, 参考指标为时间序列且为公式原型，计算指标为时间序列, 计算a, 参考b
    //-------------------------------------------------------
}

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_ref_has_alone_context_ind_is_time_without_context") {
    //-------------------------------------------------------
    // 不指定上下文, 参考指标为时间序列且为独立上下文，计算指标为时间序列, 计算a, 参考b
    //-------------------------------------------------------
}

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_ref_has_alone_context_ind_is_time_with_context") {
    //-------------------------------------------------------
    // 指定上下文, 参考指标为时间序列且为独立上下文，计算指标为时间序列, 计算a, 参考b
    //-------------------------------------------------------
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_MAVP_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_MAVP.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-40));
    Indicator x1 = TA_MAVP(kdata.close(), kdata.high());
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

    CHECK_EQ(x1.name(), x2.name());
    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t r = 0, total = x1.getResultNumber(); r < total; r++) {
        for (size_t i = x1.discard(); i < x1.size(); ++i) {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */