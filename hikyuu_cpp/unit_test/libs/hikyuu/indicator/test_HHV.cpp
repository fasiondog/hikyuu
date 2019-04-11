/*
 * test_HHV.cpp
 *
 *  Created on: 2019年4月1日
 *      Author: fasiondog
 */

#ifdef TEST_ALL_IN_ONE
    #include <boost/test/unit_test.hpp>
#else
    #define BOOST_TEST_MODULE test_hikyuu_indicator_suite
    #include <boost/test/unit_test.hpp>
#endif

#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/HHV.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_HHV test_indicator_HHV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_HHV ) {
    Indicator result;

    PriceList a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    Indicator data = PRICELIST(a);

    /** @arg n = 0 */
    result = HHV(data, 0);
    BOOST_CHECK(result.discard() == 0);
    for (int i = 0; i <10; ++i) {
        BOOST_CHECK(result[i] == data[i]);
    }

    /** @arg n = 1 */
    result = HHV(data, 1);
    BOOST_CHECK(result.discard() == 0);
    for (int i = 0; i < 10; ++i) {
        BOOST_CHECK(result[i] == data[i]);
    }

    /** @arg n = 9 */
    result = HHV(data, 9);
    BOOST_CHECK(result.discard() == 8);
    BOOST_CHECK(result[8] == data[8]);
    BOOST_CHECK(result[9] == data[9]);

    /** @arg n = 10 */
    result = HHV(data, 10);
    BOOST_CHECK(result.discard() == 9);
    BOOST_CHECK(result[9] == data[9]);
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_HHV_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/HHV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = HHV(CLOSE(kdata), 2);
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

    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


