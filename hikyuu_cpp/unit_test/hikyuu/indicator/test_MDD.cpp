/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */
#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MDD.h>

using namespace hku;

/**
 * @defgroup test_indicator_MDD test_indicator_MDD
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MDD") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, ma;

    /** @arg n = 10 且数据大小刚好为10 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    auto c = kdata.close();
    auto m = MDD(c);
    CHECK_EQ(m.name(), "MDD");
    CHECK_EQ(m.empty(), false);
    CHECK_EQ(m.size(), kdata.size());
    CHECK_EQ(m.discard(), 0);
    std::vector<price_t> expects{0.,       0.,      -0.72282, -0.60562, 0.,
                                 -3.27389, -1.0584, -2.1443,  -3.2816,  -3.5852};
    for (size_t i = 0, len = m.size(); i < len; i++) {
        CHECK_EQ(m[i], doctest::Approx(expects[i]).epsilon(0.0001));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MDD_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MDD.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator m1 = MDD(kdata.close());
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(m1);
    }

    Indicator m2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(m2);
    }

    CHECK_EQ(m2.name(), "MDD");
    CHECK_EQ(m1.size(), m2.size());
    CHECK_EQ(m1.discard(), m2.discard());
    CHECK_EQ(m1.getResultNumber(), m2.getResultNumber());
    for (size_t i = 0; i < m1.size(); ++i) {
        CHECK_EQ(m1[i], doctest::Approx(m2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
