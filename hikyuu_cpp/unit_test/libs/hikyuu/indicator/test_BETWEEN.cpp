/*
 * test_BETWEEN.cpp
 *
 *  Created on: 2019年4月2日
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
#include <hikyuu/indicator/crt/BETWEEN.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_BETWEEN test_indicator_BETWEEN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_BETWEEN ) {
    Indicator result;

    /** @arg 所有参数均为 ind */
    PriceList aval;
    for (int i = 0; i < 10; i++) {
        aval.push_back(i);
    }

    PriceList bval;
    for (int i = 0; i < 10; i++) {
        bval.push_back(5);
    }

    PriceList cval;
    for (int i = 0; i < 10; i++) {
        cval.push_back(3);
    }

    Indicator a = PRICELIST(aval);
    Indicator b = PRICELIST(bval);
    Indicator c = PRICELIST(cval);

    result = BETWEEN(a, b, c);
    BOOST_CHECK(result.name() == "BETWEEN");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result[4] == 1);
    for (int i = 0; i < 10; i++) {
        if (i != 4) {
            BOOST_CHECK(result[i] == 0);
        }
    }

    /** @arg a 为ind，b、c均为数字 */
    result = BETWEEN(a, 5, 3);
    BOOST_CHECK(result.name() == "BETWEEN");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result[4] == 1);
    for (int i = 0; i < 10; i++) {
        if (i != 4) {
            BOOST_CHECK(result[i] == 0);
        }
    }

    /** @arg a 为数字，b、c中一个为 ind，一个为数字 */
    result = BETWEEN(2, b, 2);
    BOOST_CHECK(result.name() == "BETWEEN");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == 10);
    for (int i = 0; i < 10; i++) {
        BOOST_CHECK(result[i] == 0);
    }

    result = BETWEEN(3, b, 2);
    BOOST_CHECK(result.name() == "BETWEEN");
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == 10);
    for (int i = 0; i < 10; i++) {
        BOOST_CHECK(result[i] == 1);
    }

    /** @arg a、b、c均为数字 */
    result = BETWEEN(1, 2, 3);
    BOOST_CHECK(result.name() == "BETWEEN");
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_BETWEEN_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BETWEEN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));

    PriceList aval;
    for (int i = 0; i < 10; i++) {
        aval.push_back(i);
    }

    PriceList bval;
    for (int i = 0; i < 10; i++) {
        bval.push_back(5);
    }

    PriceList cval;
    for (int i = 0; i < 10; i++) {
        cval.push_back(3);
    }

    Indicator a = PRICELIST(aval);
    Indicator b = PRICELIST(bval);
    Indicator c = PRICELIST(cval);

    Indicator x1 = BETWEEN(a, b, c);    
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


