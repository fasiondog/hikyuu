/*
 * test_export.cpp
 *
 *  Created on: 2013-4-30
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <hikyuu/config.h>

#if HKU_SUPPORT_SERIALIZATION

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/build_in.h>

using namespace hku;

/**
 * @defgroup test_indicator_weave test_indicator_serialization
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_WAEVE_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/WEAVE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator c = CLOSE(kdata);
    Indicator o = OPEN(kdata);
    Indicator w1 = WEAVE(c, o);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(w1);
    }

    Indicator w2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(w2);
    }

    BOOST_CHECK(w1.size() == w2.size());
    BOOST_CHECK(w1.discard() == w2.discard());
    BOOST_CHECK(w1.getResultNumber() == 2);
    BOOST_CHECK(w2.getResultNumber() == 2);
    size_t total = w1.size();
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK_CLOSE(w1[i], w2[i], 0.00001);
    }
    for (size_t i = 0; i < total; ++i) {
        BOOST_CHECK_CLOSE(w1.get(i,1), w2.get(i,1), 0.00001);
    }
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */

