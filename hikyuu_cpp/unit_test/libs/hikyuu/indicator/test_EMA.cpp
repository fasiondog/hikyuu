/*
 * test_EMA.cpp
 *
 *  Created on: 2013-4-10
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
#include <hikyuu/indicator/crt/EMA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>

using namespace hku;

/**
 * @defgroup test_indicator_EMA test_indicator_EMA
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_EMA ) {
    PriceList d;
    for (int i = 0; i < 30; ++i) {
        d.push_back(i);
    }

    /** @arg n = 1 */
    Indicator data = PRICELIST(d);
    Indicator ema = EMA(data, 1);
    BOOST_CHECK(ema.discard() == 0);
    BOOST_CHECK(ema.size() == 30);
    BOOST_CHECK(ema[0] == 0.0);
    BOOST_CHECK(ema[10] == 10.0);
    BOOST_CHECK(ema[29] == 29.0);

    /** @arg n = 2 */
    ema = EMA(data, 2);
    BOOST_CHECK(ema.discard() == 0);
    BOOST_CHECK(ema.size() == 30);
    BOOST_CHECK(ema[0] == 0.0);
    BOOST_CHECK(std::fabs(ema[1] - 0.66667) < 0.0001);
    BOOST_CHECK(std::fabs(ema[2] - 1.55556) < 0.0001);
    BOOST_CHECK(std::fabs(ema[3] - 2.51852) < 0.0001);
    BOOST_CHECK(std::fabs(ema[4] - 3.50617) < 0.0001);

    /** @arg n = 10 */
    ema = EMA(data, 10);
    BOOST_CHECK(ema.discard() == 0);
    BOOST_CHECK(ema.size() == 30);
    BOOST_CHECK(ema[0] == 0.0);
    BOOST_CHECK(std::fabs(ema[1] - 0.18182) < 0.0001);
    BOOST_CHECK(std::fabs(ema[2] - 0.51240) < 0.0001);

    /** @arg operator() */
    Indicator ma = MA(data, 2);
    Indicator expect = EMA(ma, 2);
    ema = EMA(2);
    Indicator result = ema(ma);
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
BOOST_AUTO_TEST_CASE( test_EMA_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/EMA.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = EMA(CLOSE(kdata), 10);
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

