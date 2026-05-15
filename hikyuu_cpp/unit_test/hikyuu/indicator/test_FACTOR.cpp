/*
 * test_FACTOR.cpp
 *
 *  Created on: 2026-05-15
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/FACTOR.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/factor/Factor.h>

using namespace hku;

/**
 * @defgroup test_indicator_FACTOR test_indicator_FACTOR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_FACTOR") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));  // 获取最近20条数据

    // 创建一个简单的MA因子用于测试
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor("TEST_FACTOR", ma5, KQuery::DAY, "测试因子", "用于测试FACTOR指标");

    /** @arg 测试FACTOR的基本功能 */
    Indicator result = FACTOR(factor);
    result.setContext(kdata);  // 设置上下文

    CHECK_EQ(result.name(), "FACTOR");
    CHECK_EQ(result.size(), kdata.size());

    // 验证结果与MA5计算结果一致
    Indicator ma5_result = MA(CLOSE(kdata), 5);
    check_indicator(result, ma5_result);

    /** @arg 测试空KData情况下FACTOR的行为 */
    KData empty_kdata;
    Indicator empty_result = FACTOR(factor);
    empty_result.setContext(empty_kdata);
    CHECK_EQ(empty_result.size(), 0);
    CHECK_EQ(empty_result.discard(), 0);

    /** @arg 测试FACTOR与KData直接结合使用 */
    Indicator direct_result = FACTOR(factor)(kdata);
    CHECK_EQ(direct_result.name(), "FACTOR");
    CHECK_EQ(direct_result.size(), kdata.size());

    // 检查直接应用的结果是否与先创建再设置上下文一致
    check_indicator(direct_result, result);
}

/** @par 检测点：测试不同类型的因子 */
TEST_CASE("test_FACTOR_different_factors") {
    Stock stock = getStock("sz000001");
    KData kdata = stock.getKData(KQuery(-10));

    // 测试不同参数的移动平均因子
    Indicator ma10 = MA(CLOSE(), 10);
    Factor factor_ma10("MA10_FACTOR", ma10, KQuery::DAY, "10日均线因子", "10日移动平均因子");

    Indicator result = FACTOR(factor_ma10);
    result.setContext(kdata);

    CHECK_EQ(result.name(), "FACTOR");
    CHECK_EQ(result.size(), kdata.size());

    // 验证结果与MA10计算结果一致
    Indicator ma10_result = MA(CLOSE(kdata), 10);
    check_indicator(result, ma10_result);

    // 测试其他类型的因子
    Indicator ma3 = MA(HIGH(), 3);
    Factor factor_high_ma3("HIGH_MA3_FACTOR", ma3, KQuery::DAY, "高点3日均线因子",
                           "高点3日移动平均因子");

    Indicator result2 = FACTOR(factor_high_ma3);
    result2.setContext(kdata);

    CHECK_EQ(result2.name(), "FACTOR");
    CHECK_EQ(result2.size(), kdata.size());

    // 验证结果与高点MA3计算结果一致
    Indicator ma3_result = MA(HIGH(kdata), 3);
    check_indicator(result2, ma3_result);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_FACTOR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/FACTOR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));

    // 创建测试因子
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor("EXPORT_TEST_FACTOR", ma5, KQuery::DAY, "导出测试因子", "用于序列化测试");

    Indicator x1 = FACTOR(factor);
    x1.setContext(kdata);

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

    CHECK_EQ(x2.name(), "FACTOR");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());

    // 验证序列化后的结果与原结果一致
    check_indicator(x1, x2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */