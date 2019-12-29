/*
 * test_MAX.cpp
 *
 *  Created on: 2019-4-9
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MAX.h>
#include <hikyuu/indicator/crt/REF.h>

using namespace hku;

/**
 * @defgroup test_indicator_MAX test_indicator_MAX
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_MAX") {
    /** @arg n > 0 */
    KData k = getStock("sh600004").getKData(KQuery(-8));
    Indicator C = CLOSE(k);
    Indicator O = OPEN(k);
    Indicator x = MAX(C, O);
    CHECK_EQ(x.name(), "MAX");
    CHECK_EQ(x.size(), 8);
    CHECK_EQ(x.discard(), 0);
    size_t total = x.size();
    for (int i = 0; i < total; i++) {
        if (C[i] > O[i]) {
            CHECK_EQ(x[i], C[i]);
        } else {
            CHECK_EQ(x[i], O[i]);
        }
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_MAX_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/MAX.xml";

    Indicator x1 = MAX(CLOSE(), OPEN());
    x1.setContext(getStock("sh600004"), KQuery(-8));

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

    CHECK_EQ(x1.name(), x2.name());
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
