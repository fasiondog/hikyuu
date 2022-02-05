/*
 * test_LLV.cpp
 *
 *  Created on: 2019年4月1日
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/LLV.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_LLV test_indicator_LLV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_LLV") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg n = 0 */
    result = LLV(data, 0);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[0]);
    }

    /** @arg n = 1 */
    result = LLV(data, 1);
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg n = 9 */
    result = LLV(data, 9);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], data[0]);
    CHECK_EQ(result[7], data[0]);
    CHECK_EQ(result[8], data[0]);
    CHECK_EQ(result[9], data[1]);

    /** @arg n = 10 */
    result = LLV(data, 10);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], data[0]);
    CHECK_EQ(result[1], data[0]);
    CHECK_EQ(result[9], data[0]);
}

/** @par 检测点 */
TEST_CASE("test_LLV_dyn") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    IndParam ind_param(PRICELIST(PriceList(10, 9)));
    result = LLV(data, ind_param);
    std::cout << result << std::endl;
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], data[0]);
    CHECK_EQ(result[7], data[0]);
    CHECK_EQ(result[8], data[0]);
    CHECK_EQ(result[9], data[1]);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_LLV_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/LLV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = LLV(CLOSE(kdata), 2);
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

    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
