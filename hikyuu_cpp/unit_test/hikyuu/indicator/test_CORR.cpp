/*
 * test_COS.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_COS test_indicator_CORR
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_CORR") {
    Indicator result;

    // 空指标
    result = CORR(Indicator(), Indicator(), 10);
    CHECK_UNARY(result.empty());

    PriceList a{0.8005901401350127,  0.6634650046370404,  -1.8000894387683657, 0.3878161916948592,
                -0.5171110201088346, -1.7905249485540904, 0.737991201931744,   0.8574053319765023,
                -0.5832251010003247, -0.1394762212893227};
    PriceList b{-0.1835983655705932, 0.02319229786147859, -0.08242549911752291,
                0.6359545988575381,  0.6421570840387792,  -0.28347907849066045,
                0.2931449663168225,  0.38137876882777144, -0.6446198144780273,
                0.2433169061114869};

    Indicator x = PRICELIST(a);
    Indicator y = PRICELIST(b);

    // 非法参数 n
    result = CORR(x, y, 0);
    CHECK_UNARY(result.empty());
    result = CORR(x, y, 1);
    CHECK_UNARY(result.empty());

    // 正常情况
    result = CORR(x, y, a.size());
    CHECK_EQ(result.name(), "CORR");
    CHECK_EQ(result.discard(), 1);
    CHECK_EQ(result.size(), a.size());

    PriceList expect_corr{Null<price_t>(),       -1.0,
                          -0.034448470909388934, 0.19152710373544807,
                          0.015928970392743255,  0.294828565718092,
                          0.3313041022900902,    0.37968156626405264,
                          0.3883899922978828,    0.3855526143087499};
    PriceList expect_cov{Null<price_t>(),     -0.014178098871419218, -0.005213202069357936,
                         0.08610594456567466, 0.006922422657103244,  0.14431515213669424,
                         0.14697313262129488, 0.15714354567421635,   0.18327891743111874,
                         0.1628944213834996};

    CHECK_UNARY(std::isnan(result[0]));
    for (int i = result.discard(); i < expect_corr.size(); ++i) {
        CHECK_EQ(result[i], doctest::Approx(expect_corr[i]).epsilon(0.00001));
    }

    Indicator cov = result.getResult(1);
    CHECK_UNARY(std::isnan(cov[0]));
    for (int i = cov.discard(); i < expect_cov.size(); ++i) {
        CHECK_EQ(cov[i], doctest::Approx(expect_cov[i]).epsilon(0.00001));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_CORR_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/COS.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = CORR(CLOSE(kdata), OPEN(kdata), 10);
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

    CHECK_EQ(x2.name(), "CORR");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = x1.discard(); i < x1.size(); ++i) {
        if (std::isnan(x1[i])) {
            CHECK_UNARY(std::isnan(x2[i]));
        } else {
            CHECK_EQ(x1[i], doctest::Approx(x2[i]));
        }
    }

    Indicator x11 = x1.getResult(1);
    Indicator x21 = x2.getResult(1);
    CHECK_EQ(x11.size(), x21.size());
    CHECK_EQ(x11.discard(), x21.discard());
    CHECK_EQ(x11.getResultNumber(), x21.getResultNumber());
    for (size_t i = x11.discard(); i < x21.size(); ++i) {
        if (std::isnan(x11[i])) {
            CHECK_UNARY(std::isnan(x21[i]));
        } else {
            CHECK_EQ(x11[i], doctest::Approx(x21[i]));
        }
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
