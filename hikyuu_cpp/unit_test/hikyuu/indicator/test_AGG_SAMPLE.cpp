/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-07-03
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/plugin/interface/plugins.h>
#include <hikyuu/plugin/extind.h>
#include <hikyuu/plugin/device.h>
#include "../plugin_valid.h"

using namespace hku;

/**
 * @defgroup test_indicator_AGG_SAMPLE test_indicator_AGG_SAMPLE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AGG_SAMPLE") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));

    /** @arg 测试默认参数（9:35采样） */
    auto ind = AGG_SAMPLE(CLOSE(), KQuery::MIN);
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_SAMPLE");
    CHECK_EQ(result.discard(), 0);

    /** @arg 验证采样值 - 找到9:35对应的分钟线数据 */
    double expected_value = 0.0;
    bool found = false;
    for (auto& kr : mink) {
        if (kr.datetime.hour() == 9 && kr.datetime.minute() == 35) {
            expected_value = kr.closePrice;
            found = true;
            break;
        }
    }
    CHECK_UNARY(found);
    CHECK_EQ(result[0], doctest::Approx(expected_value));

    /** @arg 测试指定时间参数（10:30采样） */
    ind = AGG_SAMPLE(CLOSE(), KQuery::MIN, "10:30");
    result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_SAMPLE");

    /** @arg 验证采样值 - 找到10:30对应的分钟线数据 */
    found = false;
    for (auto& kr : mink) {
        if (kr.datetime.hour() == 10 && kr.datetime.minute() == 30) {
            expected_value = kr.closePrice;
            found = true;
            break;
        }
    }
    CHECK_UNARY(found);
    CHECK_EQ(result[0], doctest::Approx(expected_value));
}

/** @par 检测点 */
TEST_CASE("test_AGG_SAMPLE_time_parameter") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111118)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));

    /** @arg 测试不同时间点采样 */
    auto ind935 = AGG_SAMPLE(CLOSE(), KQuery::MIN, "9:35");
    auto ind1000 = AGG_SAMPLE(CLOSE(), KQuery::MIN, "10:00");
    auto ind1130 = AGG_SAMPLE(CLOSE(), KQuery::MIN, "11:30");

    auto result935 = ind935(k);
    auto result1000 = ind1000(k);
    auto result1130 = ind1130(k);

    CHECK_EQ(result935.size(), k.size());
    CHECK_EQ(result1000.size(), k.size());
    CHECK_EQ(result1130.size(), k.size());

    /** @arg 验证不同时间点采样值不同 */
    double value935 = 0.0, value1000 = 0.0, value1130 = 0.0;
    bool found935 = false, found1000 = false, found1130 = false;

    for (auto& kr : mink) {
        if (kr.datetime.hour() == 9 && kr.datetime.minute() == 35) {
            value935 = kr.closePrice;
            found935 = true;
        } else if (kr.datetime.hour() == 10 && kr.datetime.minute() == 0) {
            value1000 = kr.closePrice;
            found1000 = true;
        } else if (kr.datetime.hour() == 11 && kr.datetime.minute() == 30) {
            value1130 = kr.closePrice;
            found1130 = true;
        }
    }

    CHECK_UNARY(found935);
    CHECK_UNARY(found1000);
    CHECK_UNARY(found1130);

    CHECK_EQ(result935[0], doctest::Approx(value935));
    CHECK_EQ(result1000[0], doctest::Approx(value1000));
    CHECK_EQ(result1130[0], doctest::Approx(value1130));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_AGG_SAMPLE_export") {
    HKU_IF_RETURN(!pluginValid(), void());

    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AGG_SAMPLE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = AGG_SAMPLE(CLOSE())(kdata);
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
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */