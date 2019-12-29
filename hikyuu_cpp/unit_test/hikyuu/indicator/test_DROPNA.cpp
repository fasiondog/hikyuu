/*
 * test_DROPNA.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/DROPNA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_DROPNA test_indicator_DROPNA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_DROPNA") {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg 无 nan 值 */
    result = DROPNA(data);
    CHECK_EQ(result.name(), "DROPNA");
    CHECK_EQ(result.discard(), 0);
    for (int i = 0; i < 10; ++i) {
        CHECK_EQ(result[i], data[i]);
    }

    /** @arg 全部为 nan 值 */
    a.clear();
    for (int i = 0; i < 10; i++) {
        a.push_back(Null<price_t>());
    }

    data = VALUE(a);
    result = DROPNA(data);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg 中间存在 nan 值 */
    a.push_back(Null<price_t>());
    a.push_back(12);
    a.push_back(Null<price_t>());
    a.push_back(15);
    a.push_back(Null<price_t>());
    data = VALUE(a);
    result = DROPNA(data);
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result[0], 12);
    CHECK_EQ(result[1], 15);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_DROPNA_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/DROPNA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = DROPNA(CLOSE(kdata));
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

    CHECK_EQ(x2.name(), "DROPNA");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
