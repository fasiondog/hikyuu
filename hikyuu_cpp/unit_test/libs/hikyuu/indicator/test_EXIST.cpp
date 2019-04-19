/*
 * test_EXIST.cpp
 *
 *  Created on: 2019-4-19
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
#include <hikyuu/indicator/crt/EXP.h>
#include <hikyuu/indicator/crt/EXIST.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_EXIST test_indicator_EXIST
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_EXIST ) {
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
    result = EXIST(data, 0);
    BOOST_CHECK(result.discard() == data.size());
    BOOST_CHECK(result.size() == data.size());
    
    /** @arg n=1, total>1 */
    result = EXIST(data, 1);
    BOOST_CHECK(result.name() == "EXIST");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == data.size());
    for (int i = 0; i < data.size(); ++i) {
        BOOST_CHECK(result[i] == data[i]);
    }

    /** @arg n=1, total=1 */
    result = EXIST(CVAL(1), 1);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 1);

    result = EXIST(CVAL(0), 1);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);

    /** @arg n > 1, total = n */
    result = EXIST(data, 6);
    BOOST_CHECK(result.name() == "EXIST");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 5);
    BOOST_CHECK(result[5] == 1);

    /** @arg n > 1, total < n */
    result = EXIST(data, 7);
    BOOST_CHECK(result.name() == "EXIST");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 6);

    /** @arg n > 1, total > n */
    result = EXIST(data, 3);
    BOOST_CHECK(result.name() == "EXIST");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 2);
    for (int i = 0; i < result.size(); i++) {
        std::cout << i << " " << data[i] << " " << result[i] << std::endl;
    }    
    /*BOOST_CHECK(result[2] == 1);
    BOOST_CHECK(result[3] == 1);
    BOOST_CHECK(result[4] == 1);
    BOOST_CHECK(result[5] == 1);*/

    a.push_back(0);
    a.push_back(0);
    a.push_back(0);
    a.push_back(0);

    data = PRICELIST(a);
    result = EXIST(data, 3);
    BOOST_CHECK(result.name() == "EXIST");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 2);
    /*BOOST_CHECK(result[2] == 1);
    BOOST_CHECK(result[3] == 1);
    BOOST_CHECK(result[4] == 1);
    BOOST_CHECK(result[5] == 1);*/
    
    for (int i = 0; i < result.size(); i++) {
        std::cout << i << " " << data[i] << " " << result[i] << std::endl;
    }
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_EXIST_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/EXIST.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = EXIST(CLOSE(kdata)>OPEN(kdata));
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

    BOOST_CHECK(x2.name() == "EXIST");
    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


