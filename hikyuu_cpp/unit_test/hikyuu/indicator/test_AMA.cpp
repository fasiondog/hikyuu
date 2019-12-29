/*
 * test_AMA.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_AMA test_indicator_AMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_AMA") {
    Indicator result;

    PriceList d;
    d.push_back(6063);
    d.push_back(6041);
    d.push_back(6065);
    d.push_back(6078);
    d.push_back(6114);
    d.push_back(6121);
    d.push_back(6106);
    d.push_back(6101);
    d.push_back(6166);
    d.push_back(6169);
    d.push_back(6195);
    d.push_back(6222);
    d.push_back(6186);
    d.push_back(6214);
    d.push_back(6185);
    d.push_back(6209);
    d.push_back(6221);
    d.push_back(6278);
    d.push_back(6326);
    d.push_back(6347);

    Indicator ind = PRICELIST(d);
    CHECK_EQ(ind.size(), 20);
    result = AMA(ind, 10, 2, 30);
    CHECK_EQ(result.getParam<int>("n"), 10);
    CHECK_EQ(result.getParam<int>("fast_n"), 2);
    CHECK_EQ(result.getParam<int>("slow_n"), 30);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.getResultNumber(), 2);
    CHECK_EQ(result[0], 6063);
    CHECK_LT(std::fabs(result[9] - 6103.6781), 0.000001);
    CHECK_LT(std::fabs(result[10] - 6120.760197), 0.000001);
    CHECK_LT(std::fabs(result[18] - 6216.376893), 0.000001);
    CHECK_LT(std::fabs(result[19] - 6239.100742), 0.000001);

    CHECK_EQ(result.get(0, 1), 1.0);
    CHECK_LT(std::fabs(result.get(9, 1) - 0.557895), 0.000001);
    CHECK_LT(std::fabs(result.get(10, 1) - 0.611111), 0.000001);
    CHECK_LT(std::fabs(result.get(11, 1) - 0.826484), 0.000001);
    CHECK_LT(std::fabs(result.get(18, 1) - 0.517799), 0.000001);
    CHECK_LT(std::fabs(result.get(19, 1) - 0.585526), 0.000001);

    /** @arg operator() */
    Indicator ama = AMA(10, 2, 30);
    CHECK_EQ(ama.size(), 0);
    Indicator expect = AMA(ind, 10, 2, 30);
    result = ama(ind);
    CHECK_EQ(result.size(), expect.size());
    CHECK_EQ(result.getResultNumber(), expect.getResultNumber());
    CHECK_EQ(result.discard(), expect.discard());
    for (size_t i = 0; i < expect.size(); ++i) {
        CHECK_EQ(result[i], expect[i]);
        CHECK_EQ(result.get(i, 1), expect.get(i, 1));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_AMA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/AMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = AMA(CLOSE(kdata), 10);
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
    for (size_t i = 0; i < ma1.size(); ++i) {
        CHECK_EQ(ma1.get(i, 0), doctest::Approx(ma2.get(i, 0)));
        CHECK_EQ(ma1.get(i, 1), doctest::Approx(ma2.get(i, 1)));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
