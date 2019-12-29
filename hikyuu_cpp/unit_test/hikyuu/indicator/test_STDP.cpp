/*
 * test_STDP.cpp
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/STDP.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_STDP test_indicator_STDP
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_STDP") {
    /** @arg n > 1 的正常情况 */
    PriceList d;
    for (size_t i = 0; i < 15; ++i) {
        d.push_back(i + 1);
    }
    d[5] = 4.0;
    d[7] = 4.0;
    d[11] = 6.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = STDP(ind, 10);
    CHECK_EQ(dev.name(), "STDP");
    CHECK_EQ(dev.size(), 15);
    CHECK_UNARY(std::isnan(dev[8]));
    CHECK_LT(std::fabs(dev[9] - 2.77308), 0.00001);
    CHECK_LT(std::fabs(dev[10] - 2.98161), 0.00001);
    CHECK_LT(std::fabs(dev[11] - 2.68514), 0.00001);
    CHECK_LT(std::fabs(dev[12] - 3.1), 0.00001);
    CHECK_LT(std::fabs(dev[13] - 3.46554), 0.00001);
    CHECK_LT(std::fabs(dev[14] - 3.79605), 0.00001);

    /** @arg n = 1时 */
    dev = STDP(ind, 1);
    CHECK_EQ(dev.name(), "STDP");
    CHECK_EQ(dev.size(), 15);
    for (size_t i = 0; i < dev.size(); ++i) {
        CHECK_UNARY(std::isnan(dev[i]));
    }

    /** @arg operator() */
    Indicator expect = STDP(ind, 10);
    dev = STDP(10);
    Indicator result = dev(ind);
    CHECK_EQ(result.size(), expect.size());
    for (size_t i = result.discard(); i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_STDP_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/STDP.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = STDP(CLOSE(kdata), 10);
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

    CHECK_EQ(ma2.name(), "STDP");
    CHECK_EQ(ma1.size(), ma2.size());
    CHECK_EQ(ma1.discard(), ma2.discard());
    CHECK_EQ(ma1.getResultNumber(), ma2.getResultNumber());
    for (size_t i = 0; i < ma1.discard(); i++) {
        CHECK_UNARY(std::isnan(ma2[i]));
    }
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
