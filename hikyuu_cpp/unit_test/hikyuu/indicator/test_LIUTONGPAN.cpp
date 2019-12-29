/*
 * test_LIUTONGPAN.cpp
 *
 *  Created on: 2019-3-29
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/LIUTONGPAN.h>

using namespace hku;

/**
 * @defgroup test_indicator_LIUTONGPAN test_indicator_LIUTONGPAN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_LIUTONGPAN") {
    StockManager& sm = StockManager::instance();

    Stock stk = getStock("SH600004");
    KQuery query = KQueryByDate(Datetime(200301010000), Datetime(200708250000));
    KData k = stk.getKData(query);

    Indicator liutong = LIUTONGPAN(k);
    CHECK_EQ(liutong.size(), k.size());
    size_t total = k.size();
    for (int i = 0; i < total; i++) {
        if (k[i].datetime < Datetime(200512200000)) {
            CHECK_EQ(liutong[i], 40000);
        } else if (k[i].datetime >= Datetime(200512200000) && k[i].datetime < Datetime(200612200000)) {
            CHECK_EQ(liutong[i], 47600);
        } else {
            CHECK_EQ(liutong[i], 49696);
        }
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_LIUTONGPAN_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/LIUTONGPAN.xml";

    KData k = getStock("SH600000").getKData(KQuery(-10));
    Indicator x1 = LIUTONGPAN(k);
    x1.setContext(k);

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

    CHECK_EQ(x2.name(), "LIUTONGPAN");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
