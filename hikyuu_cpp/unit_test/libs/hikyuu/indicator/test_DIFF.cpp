/*
 * test_DIFF.cpp
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
#include <hikyuu/indicator/crt/DIFF.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_DIFF test_indicator_DIFF
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_DIFF ) {
    /** @arg 正常测试 */
    PriceList d;
    for (size_t i = 0; i < 10; ++i) {
        d.push_back(i);
    }

    Indicator ind = PRICELIST(d);
    Indicator diff = DIFF(ind);
    BOOST_CHECK(diff.size() == 10);
    BOOST_CHECK(diff.discard() == 1);
    BOOST_CHECK(std::isnan(diff[0]));
    for (size_t i = 1; i < 10; ++i) {
        BOOST_CHECK(diff[i] == d[i] - d[i-1]);
    }

    /** @arg operator */
    diff = DIFF();
    Indicator expect = DIFF(ind);
    Indicator result = diff(ind);
    BOOST_CHECK(expect.size() == result.size());
    for (size_t i = 0; i < result.discard(); i++) {
        BOOST_CHECK(std::isnan(result[i]));
    }
    for (size_t i = result.discard(); i < expect.size(); ++i) {
        BOOST_CHECK(result[i] == expect[i]);
    }
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_DIFF_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/DIFF.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = DIFF(CLOSE(kdata));
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
    for (size_t i = ma1.discard(); i < ma1.size(); ++i) {
        BOOST_CHECK_CLOSE(ma1[i], ma2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


