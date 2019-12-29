/*
 * test_FILTER.cpp
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/FILTER.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_FILTER test_indicator_FILTER
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_FILTER") {
    Indicator result;

    PriceList a;
    a.push_back(0);
    a.push_back(1);
    a.push_back(0);
    a.push_back(1);
    a.push_back(1);
    a.push_back(1);
    a.push_back(1);
    a.push_back(0);

    Indicator data = PRICELIST(a);

    /** @arg n=0 */
    result = FILTER(data, 0);
    CHECK_EQ(result.name(), "FILTER");
    CHECK_EQ(result.discard(), data.size());
    CHECK_EQ(result.size(), data.size());

    /** @arg n=1, total>1 */
    result = FILTER(data, 1);
    CHECK_EQ(result.name(), "FILTER");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 1);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);

    /** @arg n=1, total=1 */
    result = FILTER(CVAL(1), 1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 1);

    result = FILTER(CVAL(0), 1);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);

    /** @arg n > 1, total = n */
    result = FILTER(data, 8);
    CHECK_EQ(result.name(), "FILTER");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);

    /** @arg n > 1, total < n */
    result = FILTER(data, 10);
    CHECK_EQ(result.name(), "FILTER");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 0);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);

    /** @arg n > 1, total > n */
    result = FILTER(data, 3);
    CHECK_EQ(result.name(), "FILTER");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);

    a.push_back(0);
    a.push_back(0);
    a.push_back(1);
    a.push_back(1);

    data = PRICELIST(a);
    result = FILTER(data, 3);
    CHECK_EQ(result.name(), "FILTER");
    CHECK_EQ(result.size(), data.size());
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 0);
    CHECK_EQ(result[1], 1);
    CHECK_EQ(result[2], 0);
    CHECK_EQ(result[3], 0);
    CHECK_EQ(result[4], 0);
    CHECK_EQ(result[5], 1);
    CHECK_EQ(result[6], 0);
    CHECK_EQ(result[7], 0);
    CHECK_EQ(result[8], 0);
    CHECK_EQ(result[9], 0);
    CHECK_EQ(result[10], 1);
    CHECK_EQ(result[11], 0);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_FILTER_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/FILTER.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = FILTER(CLOSE(kdata) > OPEN(kdata));
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

    CHECK_EQ(x2.name(), "FILTER");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
