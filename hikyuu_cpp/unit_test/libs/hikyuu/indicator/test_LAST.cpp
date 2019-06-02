/*
 * test_LAST.cpp
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
#include <hikyuu/indicator/crt/LAST.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_LAST test_indicator_LAST
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_LAST ) {
    Indicator result;

    PriceList a;
    a.push_back(0); //0
    a.push_back(1); //1
    a.push_back(2); //2
    a.push_back(3); //3 
    a.push_back(4); //4
    a.push_back(0); //5
    a.push_back(0); //6
    a.push_back(7); //7
    a.push_back(8); //8
    a.push_back(9); //9
    a.push_back(10); //10

    Indicator data = PRICELIST(a);

    /** @arg m=0, n=0 */
    result = LAST(data, 0, 0);
    BOOST_CHECK(result.name() == "LAST");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 1);
    BOOST_CHECK(result[2] == 1);
    BOOST_CHECK(result[3] == 1);
    BOOST_CHECK(result[4] == 1);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[6] == 0);
    BOOST_CHECK(result[7] == 1);
    BOOST_CHECK(result[8] == 1);
    BOOST_CHECK(result[9] == 1);
    BOOST_CHECK(result[10] == 1);

    /** @arg m=0, n=5 */
    result = LAST(data, 0, 5);
    BOOST_CHECK(result.name() == "LAST");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 5);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[6] == 0);
    BOOST_CHECK(result[7] == 0);
    BOOST_CHECK(result[8] == 0);
    BOOST_CHECK(result[9] == 0);
    BOOST_CHECK(result[10] == 0);

    /** @arg m=6,n=3*/
    result = LAST(data, 6, 3);
    BOOST_CHECK(result.name() == "LAST");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 6);
    BOOST_CHECK(result[6] == 0);
    BOOST_CHECK(result[7] == 1);
    BOOST_CHECK(result[8] == 0);
    BOOST_CHECK(result[9] == 0);
    BOOST_CHECK(result[10] == 0);

    /** @arg m=3,n=6*/
    result = LAST(data, 6, 3);
    BOOST_CHECK(result.name() == "LAST");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 6);
    BOOST_CHECK(result[6] == 0);
    BOOST_CHECK(result[7] == 1);
    BOOST_CHECK(result[8] == 0);
    BOOST_CHECK(result[9] == 0);
    BOOST_CHECK(result[10] == 0);

    /** @arg m=3,n=2*/
    result = LAST(data, 3, 2);
    BOOST_CHECK(result.name() == "LAST");
    BOOST_CHECK(result.size() == data.size()); 
    BOOST_CHECK(result.discard() == 3);
    BOOST_CHECK(result[3] == 0);
    BOOST_CHECK(result[4] == 1);
    BOOST_CHECK(result[5] == 1);
    BOOST_CHECK(result[6] == 1);
    BOOST_CHECK(result[7] == 0);
    BOOST_CHECK(result[8] == 0);
    BOOST_CHECK(result[9] == 0);
    BOOST_CHECK(result[10] == 1);

    /** @arg m=3,n=3*/
    result = LAST(data, 3, 3);
    BOOST_CHECK(result.name() == "LAST");
    BOOST_CHECK(result.size() == data.size()); 
    BOOST_CHECK(result.discard() == 3);
    BOOST_CHECK(result[3] == 0);
    BOOST_CHECK(result[4] == 1);
    BOOST_CHECK(result[5] == 1);
    BOOST_CHECK(result[6] == 1);
    BOOST_CHECK(result[7] == 1);
    BOOST_CHECK(result[8] == 0);
    BOOST_CHECK(result[9] == 0);
    BOOST_CHECK(result[10] == 1);
 }


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_LAST_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/LAST.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = LAST(CLOSE(kdata)>OPEN(kdata));
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

    BOOST_CHECK(x2.name() == "LAST");
    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


