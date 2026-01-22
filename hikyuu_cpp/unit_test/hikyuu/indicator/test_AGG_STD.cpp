/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-09-04
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
 * @defgroup test_indicator_AGG_STD test_indicator_AGG_STD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AGG_STD") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQueryByDate(Datetime(20111115)));
    auto mink =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Null<Datetime>(), KQuery::MIN));

    /** @arg 单日分钟线聚合 */
    auto ind = AGG_STD(CLOSE(), KQuery::MIN);
    auto result = ind(k);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result.name(), "AGG_STD");
    CHECK_EQ(result.discard(), 0);

    auto mink2 =
      getKData("sh000001", KQueryByDate(Datetime(20111115), Datetime(20111116), KQuery::MIN));
    double sum = 0.0;
    for (auto& kr : mink2) {
        sum += kr.closePrice;
    }
    double mean = sum / mink2.size();
    sum = 0.0;
    for (auto& kr : mink2) {
        sum += std::pow(kr.closePrice - mean, 2);
    }
    mean = std::sqrt(sum / (mink2.size() - 1));
    CHECK_EQ(result[0], doctest::Approx(mean));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_AGG_STD_export") {
    HKU_IF_RETURN(!pluginValid(), void());

    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AGG_STD.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = AGG_STD(CLOSE())(kdata);
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
