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
 * @defgroup test_indicator_AGG_SAMPLE_MIN test_indicator_AGG_SAMPLE_MIN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AGG_SAMPLE_MIN") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));

    /** @arg 测试默认参数（9:30-10:00时间段最小值） */
    auto ind = AGG_SAMPLE_MIN(CLOSE(), KQuery::MIN);
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_SAMPLE_MIN");
    CHECK_EQ(result.discard(), 0);

    /** @arg 验证时间段最小值 - 找到9:30到10:00之间的最小收盘价 */
    double expected_value = std::numeric_limits<double>::max();
    for (auto& kr : mink) {
        int hour = kr.datetime.hour();
        int minute = kr.datetime.minute();
        if ((hour == 9 && minute >= 30) || (hour == 10 && minute == 0)) {
            if (kr.closePrice < expected_value) {
                expected_value = kr.closePrice;
            }
        }
    }
    CHECK_EQ(result[0], doctest::Approx(expected_value));
}

/** @par 检测点 */
TEST_CASE("test_AGG_SAMPLE_MIN_time_range") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111118)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));

    /** @arg 测试指定时间段参数（10:00-11:00） */
    auto ind = AGG_SAMPLE_MIN(CLOSE(), KQuery::MIN, "10:00", "11:00");
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_SAMPLE_MIN");

    /** @arg 验证时间段最小值 */
    double expected_value = std::numeric_limits<double>::max();
    for (auto& kr : mink) {
        int hour = kr.datetime.hour();
        int minute = kr.datetime.minute();
        if ((hour == 10) || (hour == 11 && minute == 0)) {
            if (kr.closePrice < expected_value) {
                expected_value = kr.closePrice;
            }
        }
    }
    CHECK_EQ(result[0], doctest::Approx(expected_value));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_AGG_SAMPLE_MIN_export") {
    HKU_IF_RETURN(!pluginValid(), void());

    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AGG_SAMPLE_MIN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = AGG_SAMPLE_MIN(CLOSE())(kdata);
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