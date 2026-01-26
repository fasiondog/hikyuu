/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-29
 *      Author: fasiondog
 */

#include "../test_config.h"

#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/plugin/extind.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator_talib/ta_crt.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/ALIGN.h>
#include <hikyuu/plugin/interface/plugins.h>
#include <hikyuu/plugin/device.h>
#include "../plugin_valid.h"

using namespace hku;

/**
 * @defgroup test_indicator_WITHKTYPE test_indicator_WITHKTYPE
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_WITHKTYPE_equal_ktype") {
    HKU_IF_RETURN(!pluginValid(), void());

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

/** @par 检测点 */
TEST_CASE("test_WITHKTYPE_extent") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto k = getKData("sh000001", KQuery(-30));
    auto wk =
      getKData("sh000001", KQueryByDate(Datetime(20111021), Null<Datetime>(), KQuery::WEEK));
    Indicator ret, expect;

    ret = WITHWEEK(CLOSE());
    CHECK_EQ(ret.name(), "WITHKTYPE");
    CHECK_EQ(ret.size(), 0);
    CHECK_EQ(ret.discard(), 0);

    ret = ret(k);
    expect = ALIGN(CLOSE(wk), k, false);
    check_indicator(ret, expect);

    ret = WITHWEEK(CLOSE())(k);
    expect = ALIGN(CLOSE(wk), k, false);
    check_indicator(ret, expect);

    ret = WITHWEEK(CLOSE(k));
    expect = ALIGN(CLOSE(wk), k, false);
    check_indicator(ret, expect);

#if HKU_ENABLE_TA_LIB
    ret = WITHWEEK(TA_MA(CLOSE(), 3))(k);
    expect = ALIGN(TA_MA(CLOSE(wk), 3), k, false);
    // WITHKTYPE query 为 INDEX 索引时会夺取一些数据，会造成 discard 有所不同
    // check_indicator(ret, expect);
    for (size_t i = expect.discard(); i < ret.size(); ++i) {
        CHECK_EQ(ret[i], doctest::Approx(expect[i]).epsilon(0.00001));
    }

    wk = getKData("sh000001", KQuery(-30, Null<int64_t>(), KQuery::WEEK));
    expect = ALIGN(TA_MA(CLOSE(wk), 3), k, false);
    check_indicator(ret, expect);
#endif
}

/** @par 检测点 */
TEST_CASE("test_WITHKTYPE_sample") {
    HKU_IF_RETURN(!pluginValid(), void());

    auto wk =
      getKData("sh000001", KQueryByDate(Datetime(20110513), Datetime(20111209), KQuery::WEEK));
    auto k =
      getKData("sh000001", KQueryByDate(Datetime(20110513), Datetime(20111209), KQuery::DAY));
    Indicator ret, expect;

    ret = WITHDAY(CLOSE())(wk);
    expect = ALIGN(CLOSE(k), wk, false);
    check_indicator(ret, expect);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_WITHKTYPE_export") {
    HKU_IF_RETURN(!pluginValid(), void());
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/WITHKTYPE.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = WITHDAY(CLOSE())(kdata);

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

    CHECK_UNARY(x1.size() == x2.size());
    CHECK_UNARY(x1.discard() == x2.discard());
    CHECK_UNARY(x1.getResultNumber() == x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]).epsilon(0.00001));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
