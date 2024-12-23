/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-23
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator_talib/ta_crt.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_TA_BETA test_indicator_TA_BETA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_BETA") {
    /** @arg 非法参数 */
    CHECK_THROWS(TA_BETA(0));
    CHECK_THROWS(TA_BETA(100001));

    /** @arg 正常数据 */
    Indicator data0 = PRICELIST(PriceList{1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
    Indicator data1 = PRICELIST(PriceList{9, 10, 11, 12, 13, 14, 15, 16, 17, 18});
    double null_value = Null<price_t>();
    Indicator expect =
      PRICELIST(PriceList{null_value, null_value, null_value, null_value, null_value, 0.03926,
                          0.08285, 0.12509, 0.16500, 0.20232});

    Indicator result = TA_BETA(data0, data1, 5);
    CHECK_EQ(result.name(), "TA_BETA");
    CHECK_EQ(result.discard(), 5);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result.size(), expect.size());
    for (int i = result.discard(), len = result.size(); i < len; ++i) {
        CHECK_EQ(result[i], doctest::Approx(expect[i]).epsilon(0.00001));
    }

    /** @arg 计算数据的 discard 不为0 */
    data0 = TA_MA(getKData("sz000001", KQuery(-30)).close(), 4);
    data1 = TA_MA(getKData("sz000002", KQuery(-30)).close(), 5);
    CHECK_UNARY(data0.discard() > 0);
    CHECK_UNARY(data1.discard() > 0);
    result = TA_BETA(data0, data1);
    CHECK_EQ(result.name(), "TA_BETA");
    CHECK_EQ(result.discard(), 9);
    CHECK_EQ(result.size(), 30);
    CHECK_EQ(result[9], doctest::Approx(0.50375).epsilon(0.0001));
    CHECK_EQ(result[29], doctest::Approx(-0.12008).epsilon(0.0001));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_BETA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_BETA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_BETA(CLOSE(kdata), HIGH(kdata));
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

    CHECK_EQ(x2.name(), "TA_BETA");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
