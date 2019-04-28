/*
 * test_EVERY.cpp
 *
 *  Created on: 2019-4-28
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
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/EVERY.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_EVERY test_indicator_EVERY
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_EVERY ) {
    Indicator result;

    PriceList a;
    a.push_back(0);
    a.push_back(1);
    a.push_back(0);
    a.push_back(1);
    a.push_back(1);
    a.push_back(0);

    Indicator data = PRICELIST(a);

    /** @arg n=0 */
    result = EVERY(data, 0);
    BOOST_CHECK(result.name() == "EVERY");
    BOOST_CHECK(result.discard() == data.size()-1);
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result[5] == 0);
    for (int i = 0; i < 5; i++) {
        BOOST_CHECK(result[i] == Null<price_t>());
    }
    
    /** @arg n=1, total>1 */
    result = EVERY(data, 1);
    BOOST_CHECK(result.name() == "EVERY");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == data.size());
    for (int i = 0; i < data.size(); ++i) {
        BOOST_CHECK(result[i] == data[i]);
    }

    /** @arg n=1, total=1 */
    result = EVERY(CVAL(1), 1);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 1);

    result = EVERY(CVAL(0), 1);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);

    /** @arg n > 1, total = n */
    result = EVERY(data, 6);
    BOOST_CHECK(result.name() == "EVERY");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 5);
    BOOST_CHECK(result[5] == 0);

    /** @arg n > 1, total < n */
    result = EVERY(data, 7);
    BOOST_CHECK(result.name() == "EVERY");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 6);

    /** @arg n > 1, total > n */
    result = EVERY(data, 3);
    BOOST_CHECK(result.name() == "EVERY");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 2);
    for (int i = 0; i < result.discard(); i++) {
        BOOST_CHECK(result[i] == Null<price_t>());
    }
    BOOST_CHECK(result[2] == 0);
    BOOST_CHECK(result[3] == 0);
    BOOST_CHECK(result[4] == 0);
    BOOST_CHECK(result[5] == 0);

    a.push_back(1);
    a.push_back(1);
    a.push_back(1);
    a.push_back(1);

    data = PRICELIST(a);
    result = EVERY(data, 3);
    BOOST_CHECK(result.name() == "EVERY");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 2);
    for (int i = 0; i < result.discard(); i++) {
        BOOST_CHECK(result[i] == Null<price_t>());
    }
    BOOST_CHECK(result[2] == 0);
    BOOST_CHECK(result[3] == 0);
    BOOST_CHECK(result[4] == 0);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[6] == 0);
    BOOST_CHECK(result[7] == 0);
    BOOST_CHECK(result[8] == 1);
    BOOST_CHECK(result[9] == 1);
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_EVERY_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/EVERY.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = EVERY(CLOSE(kdata)>OPEN(kdata));
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

    BOOST_CHECK(x2.name() == "EVERY");
    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


