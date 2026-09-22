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

/** @par Test points */
TEST_CASE("test_FACTOR") {
    Stock stock = getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));  // Get the latest 20 records

    // Create a simple MA factor for the test
    Indicator ma5 = MA(CLOSE(), 5);
    Factor factor("TEST_FACTOR", ma5, KQuery::DAY, "测试因子", "用于测试FACTOR指标");

    /** @arg Test the basic functionality of FACTOR */
    Indicator result = FACTOR(factor);
    result.setContext(kdata);  // Set the context

    CHECK_EQ(result.name(), "FACTOR");
    CHECK_EQ(result.size(), kdata.size());

    // Verify that the result matches the MA5 calculation
    Indicator ma5_result = MA(CLOSE(kdata), 5);
    check_indicator(result, ma5_result);

    /** @arg Test the behavior of FACTOR with an empty KData */
    KData empty_kdata;
    Indicator empty_result = FACTOR(factor);
    empty_result.setContext(empty_kdata);
    CHECK_EQ(empty_result.size(), 0);
    CHECK_EQ(empty_result.discard(), 0);

    /** @arg Test FACTOR used directly in combination with a KData */
    Indicator direct_result = FACTOR(factor)(kdata);
    CHECK_EQ(direct_result.name(), "FACTOR");
    CHECK_EQ(direct_result.size(), kdata.size());

    // Check whether the directly applied result matches creating it and then setting the context
    check_indicator(direct_result, result);
}

/** @par Test point: test the different types of factors */
TEST_CASE("test_FACTOR_different_factors") {
    Stock stock = getStock("sz000001");
    KData kdata = stock.getKData(KQuery(-10));

    // Test the moving average factors with different parameters
    Indicator ma10 = MA(CLOSE(), 10);
    Factor factor_ma10("MA10_FACTOR", ma10, KQuery::DAY, "10日均线因子", "10日移动平均因子");

    Indicator result = FACTOR(factor_ma10);
    result.setContext(kdata);

    CHECK_EQ(result.name(), "FACTOR");
    CHECK_EQ(result.size(), kdata.size());

    // Verify that the result matches the MA10 calculation
    Indicator ma10_result = MA(CLOSE(kdata), 10);
    check_indicator(result, ma10_result);

    // Test the other types of factors
    Indicator ma3 = MA(HIGH(), 3);
    Factor factor_high_ma3("HIGH_MA3_FACTOR", ma3, KQuery::DAY, "高点3日均线因子",
                           "高点3日移动平均因子");

    Indicator result2 = FACTOR(factor_high_ma3);
    result2.setContext(kdata);

    CHECK_EQ(result2.name(), "FACTOR");
    CHECK_EQ(result2.size(), kdata.size());

    // Verify that the result matches the MA3 calculation of the high price
    Indicator ma3_result = MA(HIGH(kdata), 3);
    check_indicator(result2, ma3_result);
}

TEST_CASE("test_FACTOR_alike_uses_factor_identity") {
    Factor close_ma5("CLOSE_MA5", MA(CLOSE(), 5), KQuery::DAY);
    Factor high_ma3("HIGH_MA3", MA(HIGH(), 3), KQuery::DAY);
    Factor same_identity_different_formula("CLOSE_MA5", MA(HIGH(), 3), KQuery::DAY);
    Factor same_name_different_ktype("CLOSE_MA5", MA(CLOSE(), 5), KQuery::WEEK);

    CHECK_FALSE(FACTOR(close_ma5).alike(FACTOR(high_ma3)));
    CHECK_UNARY(FACTOR(close_ma5).alike(FACTOR(same_identity_different_formula)));
    CHECK_FALSE(FACTOR(close_ma5).alike(FACTOR(same_name_different_ktype)));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_FACTOR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/FACTOR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));

    // Create the test factor
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

    // Verify that the deserialized result matches the original one
    check_indicator(x1, x2);
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
