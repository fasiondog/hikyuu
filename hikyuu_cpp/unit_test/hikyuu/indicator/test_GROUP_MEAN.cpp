/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-15
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
 * @defgroup test_indicator_GROUP_MEAN test_indicator_GROUP_MEAN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_GROUP_MEAN") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto stk = getStock("sh000001");
    auto mink = stk.getKData(KQueryByDate(Datetime(20111115), Datetime(20111120), KQuery::MIN));

    /** @arg 分钟线按日分组 */
    auto ind = GROUP_MEAN(CLOSE(), KQuery::DAY);
    auto result = ind(mink);
    CHECK_EQ(result.name(), "GROUP_MEAN");
    CHECK_EQ(result.size(), mink.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], doctest::Approx(mink[0].closePrice));
    CHECK_EQ(result[1], doctest::Approx((mink[0].closePrice + mink[1].closePrice) / 2.0));
    CHECK_EQ(result[720], doctest::Approx(mink[720].closePrice));
    CHECK_EQ(result[721], doctest::Approx((mink[720].closePrice + mink[721].closePrice) / 2.0));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_GROUP_MEAN_export") {
    HKU_IF_RETURN(!pluginValid(), void());

    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/GROUP_MEAN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = GROUP_MEAN(CLOSE())(kdata);
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
