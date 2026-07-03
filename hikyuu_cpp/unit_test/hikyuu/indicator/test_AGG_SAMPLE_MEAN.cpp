/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-07-04
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
 * @defgroup test_indicator_AGG_SAMPLE_MEAN test_indicator_AGG_SAMPLE_MEAN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AGG_SAMPLE_MEAN") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));

    /** @arg 测试默认参数（9:30-10:00时间段平均值） */
    auto ind = AGG_SAMPLE_MEAN(CLOSE(), KQuery::MIN);
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_SAMPLE_MEAN");
    CHECK_EQ(result.discard(), 0);

    /** @arg 验证时间段平均值 - 计算9:30到10:00之间的平均收盘价 */
    double sum_value = 0.0;
    int count = 0;
    for (auto& kr : mink) {
        int hour = kr.datetime.hour();
        int minute = kr.datetime.minute();
        if ((hour == 9 && minute >= 30) || (hour == 10 && minute == 0)) {
            sum_value += kr.closePrice;
            count++;
        }
    }
    double expected_value = count > 0 ? sum_value / count : 0.0;
    CHECK_EQ(result[0], doctest::Approx(expected_value));
}

/** @par 检测点 */
TEST_CASE("test_AGG_SAMPLE_MEAN_time_range") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111118)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));

    /** @arg 测试指定时间段参数（10:00-11:00） */
    auto ind = AGG_SAMPLE_MEAN(CLOSE(), KQuery::MIN, "10:00", "11:00");
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_SAMPLE_MEAN");

    /** @arg 验证时间段平均值 */
    double sum_value = 0.0;
    int count = 0;
    for (auto& kr : mink) {
        int hour = kr.datetime.hour();
        int minute = kr.datetime.minute();
        if ((hour == 10) || (hour == 11 && minute == 0)) {
            sum_value += kr.closePrice;
            count++;
        }
    }
    double expected_value = count > 0 ? sum_value / count : 0.0;
    CHECK_EQ(result[0], doctest::Approx(expected_value));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_AGG_SAMPLE_MEAN_export") {
    HKU_IF_RETURN(!pluginValid(), void());

    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AGG_SAMPLE_MEAN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = AGG_SAMPLE_MEAN(CLOSE())(kdata);
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