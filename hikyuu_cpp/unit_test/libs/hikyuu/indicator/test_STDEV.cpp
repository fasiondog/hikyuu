/*
 * test_STD.cpp
 *
 *  Created on: 2013-4-18
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
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/STDEV.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_STDEV test_indicator_STDEV
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_STDEV ) {
    /** @arg n > 1 的正常情况 */
    PriceList d;
    for (size_t i = 0; i < 15; ++i) {
        d.push_back(i+1);
    }
    d[5] = 4.0;
    d[7] = 4.0;
    d[11] = 6.0;

    Indicator ind = PRICELIST(d);
    Indicator dev = STDEV(ind, 10);
    BOOST_CHECK(dev.size() == 15);
    BOOST_CHECK(std::isnan(dev[8]));
    BOOST_CHECK(std::fabs(dev[9] - 2.923088) < 0.000001 );
    BOOST_CHECK(std::fabs(dev[10] - 3.142893) < 0.000001 );
    BOOST_CHECK(std::fabs(dev[11] - 2.830390) < 0.000001 );
    BOOST_CHECK(std::fabs(dev[12] - 3.267686) < 0.000001 );
    BOOST_CHECK(std::fabs(dev[13] - 3.653004) < 0.000001 );
    BOOST_CHECK(std::fabs(dev[14] - 4.001388) < 0.000001 );

    /** @arg n = 1时 */
    dev = STDEV(ind, 1);
    BOOST_CHECK(dev.size() == 15);
    for (size_t i = 0; i < dev.size(); ++i) {
        BOOST_CHECK(std::isnan(dev[i]));
    }

    /** @arg operator() */
    Indicator expect = STDEV(ind, 10);
    dev = STDEV(10);
    Indicator result = dev(ind);
    BOOST_CHECK(result.size() == expect.size());
    for (size_t i = 0; i < expect.size(); ++i) {
        BOOST_CHECK(result[i] == expect[i]);
    }
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_STDEV_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/STDEV.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = STDEV(CLOSE(kdata), 10);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(ma1);
    }

    Indicator ma2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(ma2);
    }

    BOOST_CHECK(ma1.size() == ma2.size());
    BOOST_CHECK(ma1.discard() == ma2.discard());
    BOOST_CHECK(ma1.getResultNumber() == ma2.getResultNumber());
    for (size_t i = 0; i < ma1.size(); ++i) {
        BOOST_CHECK_CLOSE(ma1[i], ma2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


