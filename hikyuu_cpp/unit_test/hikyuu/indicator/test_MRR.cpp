/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MRR.h>

using namespace hku;

/**
 * @defgroup test_indicator_MRR test_indicator_MRR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MRR") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata;
    Indicator open, ma;

    /** @arg n = 10 且数据大小刚好为10 时, 正常关联数据 */
    kdata = stock.getKData(KQuery(-10));
    auto c = kdata.close();
    auto m = MRR(c);
    CHECK_EQ(m.name(), "MRR");
    CHECK_EQ(m.empty(), false);
    CHECK_EQ(m.size(), kdata.size());
    CHECK_EQ(m.discard(), 0);
    std::vector<price_t> expects{0., 0.1039, 0., 0.1181, 1.3515, 0., 2.2905, 1.1678, 0., 0.};
    for (size_t i = 0, len = m.size(); i < len; i++) {
        CHECK_EQ(m[i], doctest::Approx(expects[i]).epsilon(0.0001));
        // std::cout << c[i] << " " << m[i] << std::endl;
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MRR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MRR.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator m1 = MRR(kdata.close());
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

    CHECK_EQ(m2.name(), "MRR");
    CHECK_EQ(m1.size(), m2.size());
    CHECK_EQ(m1.discard(), m2.discard());
    CHECK_EQ(m1.getResultNumber(), m2.getResultNumber());
    for (size_t i = 0; i < m1.size(); ++i) {
        CHECK_EQ(m1[i], doctest::Approx(m2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
