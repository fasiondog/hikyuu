/*
 * test_BARSCOUNT.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 * 
 *  Created on: 2019-5-12
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
#include <hikyuu/indicator/crt/BARSLAST.h>
#include <hikyuu/indicator/crt/BARSCOUNT.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_BARSCOUNT test_indicator_BARSCOUNT
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_BARSCOUNT ) {
    Indicator result;

    /** @arg 输入指标，未关联上下文 */
    PriceList a;
    a.push_back(1);
    a.push_back(0);
    a.push_back(0);
    a.push_back(1);

    Indicator data = PRICELIST(a, 2);

    result = BARSCOUNT(data);
    BOOST_CHECK(result.name() == "BARSCOUNT");
    BOOST_CHECK(result.size() == data.size());
    BOOST_CHECK(result.discard() == 2);
    BOOST_CHECK(std::isnan(result[0]));
    BOOST_CHECK(std::isnan(result[1]));
    BOOST_CHECK(result[2] == 1);
    BOOST_CHECK(result[3] == 2);

    /** @arg 输入指标为日线 CLOSE */
    Stock stk = getStock("sz000001");
    KData k = stk.getKData(KQuery(-100));
    result = BARSCOUNT(CLOSE(k));
    BOOST_CHECK(result.name() == "BARSCOUNT");
    BOOST_CHECK(result.size() == k.size());
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 4838);
    BOOST_CHECK(result[99] == 4937);

    /** @arg 输入指标为分钟线 CLOSE */
    k = stk.getKData(KQueryByDate(Datetime(200209020000), Datetime(200209050000), KQuery::MIN));
    result = BARSCOUNT(CLOSE(k));
    BOOST_CHECK(result.name() == "BARSCOUNT");
    BOOST_CHECK(result.size() == k.size());
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result[0] == 1);
    BOOST_CHECK(result[1] == 2);
    BOOST_CHECK(result[477] == 239);
    BOOST_CHECK(result[478] == 240);
}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_BARSCOUNT_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BARSCOUNT.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = BARSCOUNT(CLOSE(kdata));
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

    BOOST_CHECK(x2.name() == "BARSCOUNT");
    BOOST_CHECK(x1.size() == x2.size());
    BOOST_CHECK(x1.discard() == x2.discard());
    BOOST_CHECK(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        BOOST_CHECK_CLOSE(x1[i], x2[i], 0.00001);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */


