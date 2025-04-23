/*
 * test_ALIGN.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#include "../test_config.h"
#include <fstream>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/ALIGN.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/PRICELIST.h>

using namespace hku;

/**
 * @defgroup test_indicator_ALIGN test_indicator_ALIGN
 * @ingroup test_hikyuu_indicator_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_ALIGN_fill_null") {
    Indicator result;
    Stock stk = getStock("sh000001");

    PriceList a;
    for (int i = 0; i < 10; i++) {
        a.push_back(i);
    }

    /** @arg 输入指标本身和上下文无关，参考日期长度为0 */
    DatetimeList ref;
    Indicator data = PRICELIST(a);
    result = ALIGN(data, ref);
    REQUIRE(result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(ref.size(), 0);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg 输入指标本身和上下文无关，和参考日期列表等长 */
    ref = stk.getDatetimeList(KQuery(-10));
    data = PRICELIST(a);
    result = ALIGN(data, ref);
    REQUIRE(result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), ref.size());
    DatetimeList result_dates = result.getDatetimeList();
    for (int i = 0; i < result.size(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg 输入指标本身和上下文无关，且长度长于参考日期列表 */
    a.push_back(11);
    data = PRICELIST(a);
    result = ALIGN(data, ref);
    REQUIRE(result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), ref.size());

    /** @arg 输入指标本身和上下文无关，且长度小于参考日期列表 */
    a.clear();
    a.push_back(1);
    data = PRICELIST(a);
    result = ALIGN(data, ref);
    REQUIRE(result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), ref.size());

    /** @arg 输入指标本身和上下文无关，且长度为0 */
    a.clear();
    data = PRICELIST(a);
    result = ALIGN(data, ref);
    REQUIRE(result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), ref.size());
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg ind对应日期全部小于参考日期 */
    KData k = stk.getKData(KQuery(-10));
    ref.clear();
    ref.push_back(Datetime(201901010000));
    ref.push_back(Datetime(201901020000));
    ref.push_back(Datetime(201901030000));
    data = CLOSE(k);
    result = ALIGN(data, ref);
    REQUIRE(result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), ref.size());
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg ind对应日期全部大于参考日期 */
    ref.clear();
    ref.push_back(Datetime(191901010000));
    ref.push_back(Datetime(191901020000));
    ref.push_back(Datetime(191901030000));
    data = CLOSE(k);
    result = ALIGN(data, ref);
    REQUIRE(result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), ref.size());
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg ind对应日期等于参考日期 */
    ref = k.getDatetimeList();
    data = CLOSE(k);
    result = ALIGN(data, ref);
    REQUIRE(result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), 0);
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.size(); i++) {
        CHECK_EQ(result[i], data[i]);
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg ind对应部分日期 */
    ref.clear();
    ref.push_back(Datetime(201111220000));
    ref.push_back(Datetime(201111230000));
    ref.push_back(Datetime(201111240000));
    ref.push_back(Datetime(201111260000));
    ref.push_back(Datetime(201112060000));
    ref.push_back(Datetime(201112070000));
    ref.push_back(Datetime(201112100000));
    data = CLOSE(k);
    result = ALIGN(data, ref);
    REQUIRE(result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), 1);
    CHECK_UNARY(std::isnan(result[0]));
    CHECK_EQ(result[1], doctest::Approx(2395.07));
    CHECK_EQ(result[2], doctest::Approx(2397.55));
    CHECK_UNARY(std::isnan(result[3]));
    CHECK_EQ(result[4], doctest::Approx(2325.91));
    CHECK_UNARY(std::isnan(result[5]));
    CHECK_UNARY(std::isnan(result[6]));
    for (int i = 0; i < result.size(); i++) {
        CHECK_EQ(result.getDatetime(i), ref[i]);
    }
}

/** @par 检测点 */
TEST_CASE("test_ALIGN_not_fill_null") {
    Indicator result;
    Stock stk = getStock("sh000001");

    PriceList a;
    for (int i = 0; i < 10; i++) {
        a.push_back(i);
    }

    /** @arg 输入指标本身和上下文无关，参考日期长度为0 */
    DatetimeList ref;
    Indicator data = PRICELIST(a);
    result = ALIGN(data, ref, false);
    REQUIRE(!result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(ref.size(), 0);
    CHECK_EQ(result.size(), 0);
    CHECK_EQ(result.discard(), 0);

    /** @arg 输入指标本身和上下文无关，和参考日期列表等长 */
    ref = stk.getDatetimeList(KQuery(-10));
    data = PRICELIST(a);
    result = ALIGN(data, ref, false);
    REQUIRE(!result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), 0);
    DatetimeList result_dates = result.getDatetimeList();
    for (int i = 0; i < result.size(); i++) {
        CHECK_EQ(result[i], data[i]);
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg 输入指标本身和上下文无关，且长度长于参考日期列表 */
    a.push_back(11);
    data = PRICELIST(a);
    result = ALIGN(data, ref, false);
    REQUIRE(!result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), 0);
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.size(); i++) {
        CHECK_EQ(result[i], data[i + 1]);
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg 输入指标本身和上下文无关，且长度小于参考日期列表 */
    a.clear();
    a.push_back(1);
    data = PRICELIST(a);
    result = ALIGN(data, ref, false);
    REQUIRE(!result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), 9);
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
        CHECK_EQ(result_dates[i], ref[i]);
    }
    CHECK_EQ(result[9], 1);
    CHECK_EQ(result_dates[9], ref[9]);

    /** @arg 输入指标本身和上下文无关，且长度为0 */
    a.clear();
    data = PRICELIST(a);
    result = ALIGN(data, ref, false);
    REQUIRE(!result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), ref.size());
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg ind对应日期全部小于参考日期 */
    KData k = stk.getKData(KQuery(-10));
    ref.clear();
    ref.push_back(Datetime(201901010000));
    ref.push_back(Datetime(201901020000));
    ref.push_back(Datetime(201901030000));
    data = CLOSE(k);
    result = ALIGN(data, ref, false);
    REQUIRE(!result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), 0);
    result_dates = result.getDatetimeList();
    Indicator::value_t expect_val = data[data.size() - 1];
    for (int i = result.discard(), len = result.size(); i < len; i++) {
        CHECK_EQ(result[i], doctest::Approx(expect_val));
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg ind对应日期全部大于参考日期 */
    ref.clear();
    ref.push_back(Datetime(191901010000));
    ref.push_back(Datetime(191901020000));
    ref.push_back(Datetime(191901030000));
    data = CLOSE(k);
    result = ALIGN(data, ref, false);
    REQUIRE(!result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), ref.size());
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.discard(); i++) {
        CHECK_UNARY(std::isnan(result[i]));
        CHECK_EQ(result_dates[i], ref[i]);
    }

    /** @arg ind对应日期等于参考日期 */
    ref = k.getDatetimeList();
    data = CLOSE(k);
    result = ALIGN(data, ref, false);
    REQUIRE(!result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), 0);
    result_dates = result.getDatetimeList();
    for (int i = 0; i < result.size(); i++) {
        CHECK_EQ(result[i], data[i]);
        CHECK_EQ(result_dates[i], ref[i]);
    }

    // /** @arg ind对应部分日期 */
    ref.clear();
    ref.push_back(Datetime(201111220000));
    ref.push_back(Datetime(201111230000));
    ref.push_back(Datetime(201111240000));
    ref.push_back(Datetime(201111260000));
    ref.push_back(Datetime(201111270000));
    ref.push_back(Datetime(201112060000));
    ref.push_back(Datetime(201112070000));
    ref.push_back(Datetime(201112100000));
    data = CLOSE(k);
    for (size_t i = 0; i < data.size(); i++) {
        HKU_INFO("{}: {}", k[i].datetime, data[i]);
    }
    result = ALIGN(data, ref, false);
    REQUIRE(!result.getParam<bool>("fill_null"));
    CHECK_EQ(result.name(), "ALIGN");
    CHECK_EQ(result.size(), ref.size());
    CHECK_EQ(result.discard(), 1);
    std::vector<Indicator::value_t> expect{
      Null<Indicator::value_t>(),
      2395.0650,
      2397.5540,
      2380.2240,
      2380.2240,
      2325.9050,
      2325.9050,
      2325.9050,
    };
    for (size_t i = result.discard(); i < result.size(); i++) {
        HKU_INFO("{} {}: {}", i, result.getDatetime(i), result[i]);
        CHECK_EQ(result.getDatetime(i), ref[i]);
        CHECK_EQ(result[i], doctest::Approx(expect[i]));
    }
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_ALIGN_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/ALIGN.xml";

    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(-20));
    Indicator x1 = ALIGN(CLOSE(kdata), sm.getStock("sh600004").getDatetimeList(KQuery(-20)));
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

    CHECK_EQ(x2.name(), "ALIGN");
    CHECK_EQ(x1.size(), x2.size());
    CHECK_EQ(x1.discard(), x2.discard());
    CHECK_EQ(x1.getResultNumber(), x2.getResultNumber());
    for (size_t i = 0; i < x1.size(); ++i) {
        CHECK_EQ(x1[i], doctest::Approx(x2[i]));
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */
