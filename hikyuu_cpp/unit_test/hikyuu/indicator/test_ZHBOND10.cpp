/*
 * test_LOG.cpp
 *
 *  Created on: 2019年4月2日
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ZHBOND10.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_indicator_ZHBOND10 test_indicator_ZHBOND10
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ZHBOND10") {
    Indicator result;

    /** @arg 输入日期列表, 未指定日期的空指标 */
    result = ZHBOND10();
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.name(), "ZHBOND10");

    /** @arg 输入日期列表, 指定日期都小于最小国债数据日期 */
    DatetimeList dates = {Datetime(19900101), Datetime(19900201)};
    result = ZHBOND10(dates);
    CHECK_EQ(result.name(), "ZHBOND10");
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < result.size(); i++) {
        CHECK_EQ(result[i], 4.0);
    }

    /** @arg 输入日期列表, 指定日期都大于最大国债数据日期 */
    dates = {Datetime(20250101), Datetime(20250201)};
    result = ZHBOND10(dates, 3.1);
    CHECK_EQ(result.name(), "ZHBOND10");
    CHECK_EQ(result.size(), 2);
    CHECK_EQ(result.discard(), 0);
    for (size_t i = 0; i < result.size(); i++) {
        CHECK_EQ(result[i], doctest::Approx(2.3375));
    }

    /** @arg 输入日期列表, 指定日期列表中对应日期断续不在收益率列表中 */
    dates = {Datetime(19900101), Datetime(19900201)};
    dates.emplace_back(20020104);
    dates.emplace_back(20020105);
    dates.emplace_back(20020106);
    dates.emplace_back(20020107);
    dates.emplace_back(20020112);
    result = ZHBOND10(dates, 3.1);
    CHECK_EQ(result.name(), "ZHBOND10");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), 7);
    CHECK_EQ(result[0], doctest::Approx(3.1));
    CHECK_EQ(result[1], doctest::Approx(3.1));
    CHECK_EQ(result[2], doctest::Approx(3.2096));
    CHECK_EQ(result[3], doctest::Approx(3.2096));
    CHECK_EQ(result[4], doctest::Approx(3.2096));
    CHECK_EQ(result[5], doctest::Approx(3.2003));
    CHECK_EQ(result[6], doctest::Approx(3.4532));

    /** @arg 输入kdata, kdata 为空 */
    KData k;
    result = ZHBOND10(k);
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.name(), "ZHBOND10");

    /** @arg 输入kdata, kdata 为日线不为空 */
    k = getKData("sh000001", KQueryByDate(Datetime(20011226), Datetime(20020111)));
    result = ZHBOND10(k, 3.0);
    CHECK_EQ(result.name(), "ZHBOND10");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result[0], doctest::Approx(3.0));
    CHECK_EQ(result[1], doctest::Approx(3.0));
    CHECK_EQ(result[2], doctest::Approx(3.0));
    CHECK_EQ(result[3], doctest::Approx(3.0));
    CHECK_EQ(result[4], doctest::Approx(3.2096));
    CHECK_EQ(result[5], doctest::Approx(3.2003));
    CHECK_EQ(result[6], doctest::Approx(3.5225));
    CHECK_EQ(result[7], doctest::Approx(3.5896));
    CHECK_EQ(result[8], doctest::Approx(3.5784));

    /** @arg 输入 Indicator, Indicator 为空 */
    Indicator ind;
    result = ZHBOND10(ind);
    CHECK_UNARY(result.empty());
    CHECK_EQ(result.name(), "ZHBOND10");

    /** @arg 输入 Indicator 不为空 */
    k = getKData("sh000001", KQueryByDate(Datetime(20011226), Datetime(20020111)));
    result = ZHBOND10(k.close(), 3.0);
    CHECK_EQ(result.name(), "ZHBOND10");
    CHECK_EQ(result.discard(), 0);
    CHECK_EQ(result.size(), k.size());
    CHECK_EQ(result[0], doctest::Approx(3.0));
    CHECK_EQ(result[1], doctest::Approx(3.0));
    CHECK_EQ(result[2], doctest::Approx(3.0));
    CHECK_EQ(result[3], doctest::Approx(3.0));
    CHECK_EQ(result[4], doctest::Approx(3.2096));
    CHECK_EQ(result[5], doctest::Approx(3.2003));
    CHECK_EQ(result[6], doctest::Approx(3.5225));
    CHECK_EQ(result[7], doctest::Approx(3.5896));
    CHECK_EQ(result[8], doctest::Approx(3.5784));
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_ZHBOND10_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ZHBOND10.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ZHBOND10(kdata);
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
