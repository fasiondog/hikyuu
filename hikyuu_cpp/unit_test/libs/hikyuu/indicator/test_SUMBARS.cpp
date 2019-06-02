/*
 * test_SUMBARS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-5-7
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
#include <hikyuu/indicator/crt/SUMBARS.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_SUMBARS test_indicator_SUMBARS
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_SUMBARS ) {
    Indicator result;

    PriceList a;

    for (int i = 0; i < 10; i++) {
        a.push_back(i * 10);
    }
    Indicator data = PRICELIST(a);

    result = SUMBARS(data, 10);
    BOOST_CHECK(result.name() == "SUMBARS");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 1);
    BOOST_CHECK(std::isnan(result[0]));
    BOOST_CHECK(result[1] == 0);
    BOOST_CHECK(result[2] == 0);
    BOOST_CHECK(result[3] == 0);
    BOOST_CHECK(result[4] == 0);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[6] == 0);
    BOOST_CHECK(result[7] == 0);
    BOOST_CHECK(result[8] == 0);
    BOOST_CHECK(result[9] == 0);

    result = SUMBARS(data, 15);
    BOOST_CHECK(result.name() == "SUMBARS");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 2);
    BOOST_CHECK(std::isnan(result[0]));
    BOOST_CHECK(std::isnan(result[1]));
    BOOST_CHECK(result[2] == 0);
    BOOST_CHECK(result[3] == 0);
    BOOST_CHECK(result[4] == 0);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[6] == 0);
    BOOST_CHECK(result[7] == 0);
    BOOST_CHECK(result[8] == 0);
    BOOST_CHECK(result[9] == 0);

    result = SUMBARS(data, 90);
    BOOST_CHECK(result.name() == "SUMBARS");
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 4);
    BOOST_CHECK(std::isnan(result[0]));
    BOOST_CHECK(std::isnan(result[1]));
    BOOST_CHECK(std::isnan(result[2]));
    BOOST_CHECK(std::isnan(result[3]));
    BOOST_CHECK(result[4] == 2);
    BOOST_CHECK(result[5] == 1);
    BOOST_CHECK(result[6] == 1);
    BOOST_CHECK(result[7] == 1);
    BOOST_CHECK(result[8] == 1);
    BOOST_CHECK(result[9] == 0);

    result = SUMBARS(data, 0);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result[1] == 0);
    BOOST_CHECK(result[2] == 0);
    BOOST_CHECK(result[3] == 0);
    BOOST_CHECK(result[4] == 0);
    BOOST_CHECK(result[5] == 0);
    BOOST_CHECK(result[6] == 0);
    BOOST_CHECK(result[7] == 0);
    BOOST_CHECK(result[8] == 0);
    BOOST_CHECK(result[9] == 0);

    result = SUMBARS(data, 170);
    BOOST_CHECK(result.discard() == 6);
    BOOST_CHECK(result.size() == 10);
    for (size_t i = 0; i < result.discard(); i++) {
        BOOST_CHECK(std::isnan(result[i]));
    }
    BOOST_CHECK(result[6] == 3);
    BOOST_CHECK(result[7] == 2);
    BOOST_CHECK(result[8] == 2);
    BOOST_CHECK(result[9] == 1);

    result = SUMBARS(data, 450);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 9);
    for (size_t i = 0; i < result.discard(); i++) {
        BOOST_CHECK(std::isnan(result[i]));
    }
    BOOST_CHECK(result[9] == 8);

    result = SUMBARS(data, 451);
    BOOST_CHECK(result.size() == 10);
    BOOST_CHECK(result.discard() == 10);
    for (size_t i = 0; i < result.discard(); i++) {
        BOOST_CHECK(std::isnan(result[i]));
    }

    result = SUMBARS(CVAL(10), 451);
    BOOST_CHECK(result.discard() == 1);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(std::isnan(result[0]));

    result = SUMBARS(CVAL(10), 10);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);

    a.clear();
    a.push_back(10);
    a.push_back(20);
    data = PRICELIST(a);
    result = SUMBARS(data, 20);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result.discard() == 1);
    BOOST_CHECK(std::isnan(result[0]));
    BOOST_CHECK(result[1] == 0);

    result = SUMBARS(data, 0);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 0);
    BOOST_CHECK(result[1] == 0);

    result = SUMBARS(data, 30);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result.discard() == 1);
    BOOST_CHECK(std::isnan(result[0]));
    BOOST_CHECK(result[1] == 1);

    result = SUMBARS(data, 40);
    BOOST_CHECK(result.size() == 2);
    BOOST_CHECK(result.discard() == 2);
    BOOST_CHECK(std::isnan(result[0]));
    BOOST_CHECK(std::isnan(result[1]));
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_SUMBARS_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SUMBARS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = SUMBARS(CLOSE(kdata), 10000);
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

    BOOST_CHECK(x2.name() == "SUMBARS");
    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


