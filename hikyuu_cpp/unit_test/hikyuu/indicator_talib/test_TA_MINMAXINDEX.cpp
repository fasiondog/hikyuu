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
 * @defgroup test_indicator_TA_MINMAXINDEX test_indicator_TA_MINMAXINDEX
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_TA_MINMAXINDEX") {
    /** @arg 非法参数 */
    CHECK_THROWS(TA_MINMAXINDEX(1));
    CHECK_THROWS(TA_MINMAXINDEX(100001));

    /** @arg 正常数据 */
    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);
    Indicator expect0 = PRICELIST(
      PriceList{Null<Indicator::value_t>(), Null<Indicator::value_t>(), 0, 1, 2, 3, 4, 5, 6, 7});
    Indicator expect1 = PRICELIST(
      PriceList{Null<Indicator::value_t>(), Null<Indicator::value_t>(), 2, 3, 4, 5, 6, 7, 8, 9});

    Indicator result = TA_MINMAXINDEX(data, 3);
    CHECK_EQ(result.name(), "TA_MINMAXINDEX");
    CHECK_EQ(result.discard(), 2);
    CHECK_EQ(result.getResultNumber(), 2);
    CHECK_EQ(result.size(), expect0.size());
    for (int i = result.discard(), len = result.size(); i < len; ++i) {
        CHECK_EQ(result[i], expect0[i]);
        CHECK_EQ(result.get(i, 1), expect1[i]);
    }

    /** @arg 计算数据的 discard 不为0 */
    data = TA_MA(getKData("sh000001", KQuery(-10)).close(), 3);
    CHECK_EQ(data.discard(), 2);
    result = TA_MINMAXINDEX(data, 3);
    CHECK_EQ(result.name(), "TA_MINMAXINDEX");
    CHECK_EQ(result.discard(), 4);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result[4], 3.);
    CHECK_EQ(result[9], 9.);
    CHECK_EQ(result.get(4, 1), 4.);
    CHECK_EQ(result.get(9, 1), 7.);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_TA_MINMAXINDEX_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TA_MINMAXINDEX.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = TA_MINMAXINDEX(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "TA_MINMAXINDEX");
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
