/*
 * test_CVAL.cpp
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_CVAL test_indicator_CVAL
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_CVAL") {
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

    /** @arg operator(ind) */
    Indicator ind = PRICELIST(d);
    CHECK_EQ(ind.size(), 20);
    result = CVAL(ind, 100);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.getResultNumber(), 1);
    for (size_t i = 0; i < ind.size(); ++i) {
        CHECK_EQ(result[i], 100);
    }

    /** @arg operator() */
    result = CVAL(100);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_EQ(result[0], 100);

    /** @arg 测试discard, 未指定ind discard=2 */
    result = CVAL(100, 2);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.getParam<int>("discard"), 2);
    CHECK_EQ(result.size(), 1);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.getResultNumber(), 1);
    CHECK_UNARY(std::isnan(result[0]));

    /** @arg 测试discard, ind discard=2 */
    ind = PRICELIST(d);
    CHECK_EQ(ind.size(), 20);
    result = CVAL(ind, 100, 2);
    CHECK_EQ(result.getParam<double>("value"), 100);
    CHECK_EQ(result.getParam<int>("discard"), 2);
    CHECK_EQ(result.size(), 20);
    CHECK_EQ(result.empty(), false);
    CHECK_EQ(result.discard(), 2);
    CHECK_EQ(result.getResultNumber(), 1);
    for (size_t i = result.discard(); i < ind.size(); ++i) {
        CHECK_EQ(result[i], 100);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_CVAL_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/CVAL.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = CVAL(CLOSE(kdata), 100.0);
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
        CHECK_EQ(ma1[i], doctest::Approx(ma2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
