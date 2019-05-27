/*
 * test_CVAL.cpp
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
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
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_CVAL test_indicator_CVAL
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_CVAL ) {
    Indicator result;

    PriceList d;
    d.push_back(6063);
    d.push_back(6041);
    d.push_back(6065);
    d.push_back(6078);
    d.push_back(6114);
    d.push_back(6121);
    d.push_back(6106);
    d.push_back(6101);
    d.push_back(6166);
    d.push_back(6169);
    d.push_back(6195);
    d.push_back(6222);
    d.push_back(6186);
    d.push_back(6214);
    d.push_back(6185);
    d.push_back(6209);
    d.push_back(6221);
    d.push_back(6278);
    d.push_back(6326);
    d.push_back(6347);

    /** @arg operator(ind) */
    Indicator ind = PRICELIST(d);
    BOOST_CHECK(ind.size() == 20);
    result = CVAL(ind, 100);
    BOOST_CHECK(result.getParam<double>("value") == 100);
    BOOST_CHECK(result.size() == 20);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.getResultNumber() == 1);
    for (size_t i = 0; i < ind.size(); ++i) {
        BOOST_CHECK(result[i] == 100);
    }

    /** @arg operator() */
    result = CVAL(100);
    BOOST_CHECK(result.getParam<double>("value") == 100);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.discard() == 0);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(result[0] == 100);

    /** @arg 测试discard, 未指定ind discard=2 */
    result = CVAL(100, 2);
    BOOST_CHECK(result.getParam<double>("value") == 100);
    BOOST_CHECK(result.getParam<int>("discard") == 2);
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.discard() == 1);
    BOOST_CHECK(result.getResultNumber() == 1);
    BOOST_CHECK(std::isnan(result[0]));

    /** @arg 测试discard, ind discard=2 */
    ind = PRICELIST(d);
    BOOST_CHECK(ind.size() == 20);
    result = CVAL(ind, 100, 2);
    BOOST_CHECK(result.getParam<double>("value") == 100);
    BOOST_CHECK(result.getParam<int>("discard") == 2);
    BOOST_CHECK(result.size() == 20);
    BOOST_CHECK(result.empty() == false);
    BOOST_CHECK(result.discard() == 2);
    BOOST_CHECK(result.getResultNumber() == 1);
    for (size_t i = result.discard(); i < ind.size(); ++i) {
        BOOST_CHECK(result[i] == 100);
    }

}


//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
BOOST_AUTO_TEST_CASE( test_CVAL_export ) {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/CVAL.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator ma1 = CVAL(CLOSE(kdata), 100.0);
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


