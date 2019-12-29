/*
 * test_DIFF.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/DIFF.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_DIFF test_indicator_DIFF
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_DIFF") {
    /** @arg 正常测试 */
    PriceList d;
    for (size_t i = 0; i < 10; ++i) {
        d.push_back(i);
    }

    Indicator ind = PRICELIST(d);
    Indicator diff = DIFF(ind);
    CHECK_EQ(diff.size(), 10);
    CHECK_EQ(diff.discard(), 1);
    CHECK_UNARY(std::isnan(diff[0]));
    for (size_t i = 1; i < 10; ++i) {
        CHECK_EQ(diff[i], d[i] - d[i - 1]);
    }

    /** @arg operator */
    diff = DIFF();
    Indicator expect = DIFF(ind);
    Indicator result = diff(ind);
    CHECK_EQ(expect.size(), result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_DIFF_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/DIFF.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = DIFF(CLOSE(kdata));
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
