/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-26
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator_talib/ta_crt.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_TA_MAVP test_indicator_TA_MAVP
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_MAVP") {
    KData k1 = getKData("sz000001", KQuery(-35));
    KData k2 = getKData("sz000002", KQuery(-35));
    Indicator result, a, b;

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

    //-------------------------------------------------------
    // 输入序列均为时间无关序列，计算a, 参考b
    //-------------------------------------------------------

    /** @arg a,b 长度都为1 (长度相等且小于2)，不指定上下文 */
    a = CVAL(1.);
    b = CVAL(2.);
    CHECK_EQ(a.size(), 1);
    CHECK_EQ(b.size(), 1);
    result = TA_MAVP(a, b);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.name(), "TA_MAVP(CVAL)");
    CHECK_UNARY(std::isnan(result[0]));

    /** @arg a, b 长度都为1 (长度相等且小于2)，指定上下文 */
    a = CVAL(1.);
    b = CVAL(2.);
    CHECK_EQ(a.size(), 1);
    CHECK_EQ(b.size(), 1);
    result = TA_MAVP(a, b)(k1);
    CHECK_EQ(result.size(), 35);
    CHECK_EQ(result.discard(), 35);
    CHECK_EQ(result.name(), "TA_MAVP(CVAL)");
    for (size_t i = 0, len = result.discard(); i < len; i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }

    /** @arg a 长度为0，b 长度为1，不指定上下文 */
    a = PRICELIST();
    b = CVAL(1.);
    CHECK_EQ(a.size(), 0);
    CHECK_EQ(b.size(), 1);
    result = TA_MAVP(a, b);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg a 长度为0，b 长度为1，指定上下文 */
    a = PRICELIST();
    b = CVAL(1.);
    CHECK_EQ(a.size(), 0);
    CHECK_EQ(b.size(), 1);
    result = TA_MAVP(a, b)(k1);
    CHECK_EQ(result.size(), 35);
    CHECK_EQ(result.discard(), 35);

    //-------------------------------------------------------
    // 参考指标为时间无关序列，计算指标为时间序列
    //-------------------------------------------------------

    //-------------------------------------------------------
    // 参考指标为时间序列且为公式原型，计算指标为时间序列
    //-------------------------------------------------------

    //-------------------------------------------------------
    // 参考指标为时间序列且为独立上下文，计算指标为时间序列
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