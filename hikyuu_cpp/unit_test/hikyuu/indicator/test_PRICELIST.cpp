/*
 * test_PRICELIST.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_PRICELIST test_indicator_PRICELIST
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_PRICELIST") {
    PriceList tmp_list;
    Indicator result;

    /** @arg PriceList 为空 */
    result = PRICELIST(tmp_list);
    CHECK_EQ(result.size(), tmp_list.size());
    CHECK_EQ(result.empty(), true);
    /** @arg PriceList 非空 */
    for (size_t i = 0; i < 10; ++i) {
        tmp_list.push_back(i);
    }
    result = PRICELIST(tmp_list);
    CHECK_EQ(result.size(), tmp_list.size());
    CHECK_EQ(result.empty(), false);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], tmp_list[i]);
    }

    /** @arg 数组指针为空 */
    price_t* p_tmp = NULL;
    result = PRICELIST(p_tmp, 10);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.empty(), true);

    /** @arg 数组指针非空 */
    price_t tmp[10];
    for (size_t i = 0; i < 10; ++i) {
        tmp[i] = i;
    }
    result = PRICELIST(tmp, 10);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], tmp[i]);
    }

    /** @arg 从PriceList构造 */
    result = PRICELIST(tmp_list);
    CHECK_EQ(result.size(), 10);
    for (size_t i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], tmp_list[i]);
    }

    /** @arg 从PriceLIst， discard为1 */
    result = PRICELIST(tmp_list, 1);
    CHECK_EQ(result.size(), 10);
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    for (size_t i = 1; i < 10; ++i) {
        CHECK_EQ(result[i], tmp_list[i]);
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_PRICELIST_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/PRICELIST.xml";

    PriceList d;
    for (size_t i = 0; i < 20; ++i) {
        d.push_back(i);
    }

    Indicator ma1 = PRICELIST(d);
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
