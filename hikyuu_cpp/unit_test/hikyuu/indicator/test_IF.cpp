/*
 * test_IF.cpp
 *
 *  Created on: 2019-3-30
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/REF.h>

using namespace hku;

/**
 * @defgroup test_indicator_IF test_indicator_IF
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_IF") {
    KData kdata = getStock("SH600000").getKData(KQuery(-10));

    /** @arg 三个参数均为 indicator */
    Indicator x = IF(CLOSE() > OPEN(), CVAL(1), CVAL(0));
    x.setContext(kdata);
    Indicator c = CLOSE(kdata);
    Indicator o = OPEN(kdata);
    for (int i = 0; i < x.size(); i++) {
        if (c[i] > o[i]) {
            CHECK_EQ(x[i], 1);
        } else {
            CHECK_EQ(x[i], 0);
        }
    }

    /** @arg 测试调用operator()(const Indicator&) */
    x = IF(REF(0) > REF(1), 1, 0);
    c = CLOSE(kdata);
    x = x(c);
    for (int i = 0; i < x.size(); i++) {
        if (i == 0)
            CHECK(std::isnan(x[i]));
        else
            CHECK_EQ(x[i], c[i] > c[i - 1] ? 1 : 0);
    }

    /** @arg 参数其中之一为数字 */
    x = IF(CLOSE() > OPEN(), 1, CVAL(0));
    x.setContext(kdata);
    for (int i = 0; i < x.size(); i++) {
        if (c[i] > o[i]) {
            CHECK_EQ(x[i], 1);
        } else {
            CHECK_EQ(x[i], 0);
        }
    }

    x = IF(CLOSE() > OPEN(), CVAL(1), 0);
    x.setContext(kdata);
    for (int i = 0; i < x.size(); i++) {
        if (c[i] > o[i]) {
            CHECK_EQ(x[i], 1);
        } else {
            CHECK_EQ(x[i], 0);
        }
    }

    /** @arg 两个参数为数字 */
    x = IF(CLOSE() > OPEN(), 1, 0);
    x.setContext(kdata);
    for (int i = 0; i < x.size(); i++) {
        if (c[i] > o[i]) {
            CHECK_EQ(x[i], 1);
        } else {
            CHECK_EQ(x[i], 0);
        }
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_IF_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/IF.xml";

    KData kdata = getStock("SH600000").getKData(KQuery(-10));
    Indicator x1 = IF(CLOSE() > OPEN(), CVAL(1), CVAL(0));
    x1.setContext(kdata);

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

    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
