/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-29
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/WITHKTYPE.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/ALIGN.h>

using namespace hku;

/**
 * @defgroup test_indicator_WITHKTYPE test_indicator_WITHKTYPE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_WITHKTYPE_equal_ktype") {
    auto k = getKData("sh000001", KQuery(-30));
    Indicator ret;

    ret = WITHDAY(CLOSE());
    CHECK_EQ(ret.name(), "WITHKTYPE");
    CHECK_EQ(ret.size(), 0);
    CHECK_EQ(ret.discard(), 0);

    ret = WITHDAY(CLOSE(k));
    check_indicator(ret, k.close());

    ret = ret(k);
    check_indicator(ret, k.close());

    ret = WITHDAY(CLOSE())(k);
    check_indicator(ret, k.close());

    auto wk = getKData("sh000001", KQuery(-30, Null<int64_t>(), KQuery::WEEK));

    ret = WITHWEEK(CLOSE());
    CHECK_EQ(ret.name(), "WITHKTYPE");
    CHECK_EQ(ret.size(), 0);
    CHECK_EQ(ret.discard(), 0);

    ret = WITHWEEK(CLOSE(wk));
    check_indicator(ret, wk.close());

    ret = ret(wk);
    check_indicator(ret, wk.close());

    ret = WITHWEEK(CLOSE())(wk);
    check_indicator(ret, wk.close());
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_WITHKTYPE_export") {
    // StockManager& sm = StockManager::instance();
    // string filename(sm.tmpdir());
    // filename += "/WITHKTYPE.xml";

    // Stock stock = sm.getStock("sh000001");
    // KData kdata = stock.getKData(KQuery(-20));
    // Indicator x1 = hku::WITHKTYPE(MA(CLOSE(kdata), 3));
    // {
    //     std::ofstream ofs(filename);
    //     boost::archive::xml_oarchive oa(ofs);
    //     oa << BOOST_SERIALIZATION_NVP(x1);
    // }

    // Indicator x2;
    // {
    //     std::ifstream ifs(filename);
    //     boost::archive::xml_iarchive ia(ifs);
    //     ia >> BOOST_SERIALIZATION_NVP(x2);
    // }

    // CHECK_UNARY(x1.size() == x2.size());
    // CHECK_UNARY(x1.discard() == x2.discard());
    // CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    // for (size_t i = 0; i < x1.size(); ++i) {
    //     CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    // }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
