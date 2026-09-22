/*
 * test_Parameter.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include "hikyuu/utilities/Log.h"
#include <hikyuu/utilities/Parameter.h>
#include <hikyuu/StockManager.h>

#if HKU_SUPPORT_SERIALIZATION
#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif

using namespace hku;

/**
 * @defgroup test_hikyuu_Parameter test_hikyuu_Parameter
 * @ingroup test_hikyuu_utilities
 * @{
 */

/** @par Test points */
TEST_CASE("test_Parameter") {
    Parameter param;

    /** @arg The normal adding, reading and modifying of the parameters */
    param.set<int>("n", 1);
    param.set<int64_t>("n64", 21474836480ll);
    param.set<bool>("bool", true);
    param.set<double>("double", 10);
    param.set<string>("string", "test");
    param.set<Datetime>("datetime", Datetime(2023, 1, 1));
    CHECK(param.get<int>("n") == 1);
    CHECK(param.get<int64_t>("n64") == 21474836480ll);
    CHECK(param.get<bool>("bool") == true);
    CHECK(param.get<double>("double") == 10.0);
    CHECK(param.get<string>("string") == "test");
    CHECK(param.get<Datetime>("datetime") == Datetime(2023, 1, 1));
    param.set<int>("n", 10);
    param.set<int64_t>("n64", -21474836480ll);
    param.set<bool>("bool", false);
    param.set<double>("double", 10.01);
    param.set<string>("string", "test2");
    CHECK(param.get<int>("n") == 10);
    CHECK(param.get<int64_t>("n64") == -21474836480ll);
    CHECK(param.get<bool>("bool") == false);
    CHECK(param.get<double>("double") == 10.01);
    CHECK(param.get<string>("string") == "test2");

    /** @arg Add an unsupported parameter type */
    CHECK_THROWS_AS(param.set<size_t>("n", 10), std::logic_error);
    CHECK_THROWS_AS(param.set<float>("n", 10.0), std::logic_error);

    /** @arg When modifying a parameter, the given type does not match the existing one */
    CHECK_THROWS_AS(param.set<float>("n", 10.0), std::logic_error);
    CHECK_THROWS_AS(param.set<float>("bool", 10.0), std::logic_error);
    CHECK_THROWS_AS(param.set<float>("double", 10.0), std::logic_error);

    /** @arg Test the equality comparison */
    Parameter p1, p2;
    p1.set<string>("string", "test");
    p1.set<bool>("bool", true);
    p1.set<double>("double", 0.01);
    p1.set<string>("test", "test2");

    p2.set<double>("double", 0.01);
    p2.set<string>("test", "test2");
    p2.set<string>("string", "test");
    p2.set<bool>("bool", true);

    CHECK_EQ(p1, p2);

    p2.set<double>("double", 1.0);
    CHECK(p1 != p2);

    p2.set<double>("double", 0.01);
    CHECK_EQ(p1, p2);
    p2.set<string>("string", "n");
    CHECK(p1 != p2);

    p2.set<string>("string", "test");
    CHECK_EQ(p1, p2);
    p2.set<int>("new", 1);
    CHECK(p1 != p2);

    p1.set<int>("new", 1);
    CHECK_EQ(p1, p2);
    p1.set<PriceList>("ps", {1.0, 2.0});
    CHECK(p1 != p2);
    p2.set<PriceList>("ps", {1.0, 2.0});
    CHECK_EQ(p1, p2);
    p2.set<PriceList>("ps", {1.0, 3.0});
    CHECK(p1 != p2);

    p2.set<PriceList>("ps", {1.0, 2.0});
    CHECK_EQ(p1, p2);

    /** @arg Test using Stock as a parameter */
    Stock stk = getStock("sh600000");
    Parameter p;
    p.set<Stock>("stk", stk);
    Stock stk2 = p.get<Stock>("stk");
    CHECK(stk == stk2);

    /** @arg Test using Block as a parameter */
    Block blk;
    p = Parameter();
    p.set<Block>("blk", blk);
    Block blk2 = p.get<Block>("blk");
    CHECK(blk == blk2);

    /** @arg Test using KQuery as a parameter */
    KQuery query(10, 20);
    p = Parameter();
    p.set<KQuery>("query", query);
    KQuery q2;
    q2 = p.get<KQuery>("query");
    CHECK(query == q2);

    /** @arg Test using KData as a parameter */
    KData k = stk.getKData(query);
    p = Parameter();
    p.set<KData>("k", k);
    KData k2 = p.get<KData>("k");
    CHECK(k.size() == k2.size());
    CHECK(k.getStock() == k2.getStock());
    CHECK(k.getQuery() == k2.getQuery());

    k = KData();
    p = Parameter();
    p.set<KData>("k", k);
    k2 = p.get<KData>("k");
    CHECK(k.size() == k2.size());
    CHECK(k.getStock() == k2.getStock());
    CHECK(k.getQuery() == k2.getQuery());

    /** @arg Test using PriceList as a parameter */
    PriceList x;
    for (int i = 0; i < 10; i++) {
        x.push_back(i);
    }
    p = Parameter();
    p.set<PriceList>("x", x);
    PriceList x2 = p.get<PriceList>("x");
    CHECK(x.size() == x2.size());
    for (int i = 0; i < 10; i++) {
        CHECK(x[i] == x2[i]);
    }
}

/** @par Verify the getting of KData */
TEST_CASE("test_Parameter_KData") {
    KData k = getKData("sh000001", KQuery(-10));
    CHECK_EQ(k.size(), 10);

    Parameter param;
    param.set<KData>("k", k);

    /** @arg Verify that KData can be read several times, avoiding the move semantics */
    auto ek = param.get<KData>("k");
    CHECK_EQ(ek, k);

    auto ek2 = param.get<KData>("k");
    CHECK_EQ(ek2, k);
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Test points */
TEST_CASE("test_Parameter_serialize") {
    string filename(StockManager::instance().tmpdir());
    filename += "/Parameter.xml";

    Parameter p1;
    p1.set<int>("n", 10);
    p1.set<bool>("bool", true);
    p1.set<double>("p", 0.101);
    p1.set<string>("string", "This is string!");
    p1.set<Datetime>("date", Datetime(20260101));
    Stock stk = getStock("sh600000");
    KQuery q = KQueryByDate(Datetime(200001041025), Datetime(200001041100), KQuery::MIN5);
    KData k = stk.getKData(q);
    DatetimeList d = k.getDatetimeList();
    p1.set<Stock>("stk", stk);
    p1.set<Block>("blk", Block());
    p1.set<KQuery>("query", q);
    p1.set<KData>("kdata", k);
    p1.set<DatetimeList>("dates", d);

    PriceList x;
    for (int i = 0; i < 10; i++) {
        x.push_back(i);
    }
    p1.set<PriceList>("x", x);

    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(p1);
    }

    Parameter p2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(p2);
    }

    CHECK(p2.get<int>("n") == 10);
    CHECK(p2.get<bool>("bool") == true);
    CHECK(p2.get<double>("p") == 0.101);
    CHECK(p2.get<string>("string") == "This is string!");
    CHECK(p2.get<Stock>("stk") == stk);
    CHECK(p2.get<Block>("blk") == Block());
    CHECK(p2.get<KQuery>("query") == q);
    CHECK(p2.get<Datetime>("date") == Datetime(20260101));
    KData k2 = p2.get<KData>("kdata");
    CHECK(k.size() == k2.size());
    CHECK(k.getStock() == k2.getStock());
    CHECK(k.getQuery() == k2.getQuery());
    PriceList x2 = p2.get<PriceList>("x");
    CHECK(x.size() == x2.size());
    for (int i = 0; i < 10; i++) {
        CHECK(x[i] == x2[i]);
    }
    DatetimeList d2 = p2.get<DatetimeList>("dates");
    CHECK(d.size() == k.size());
    CHECK(d.size() == d2.size());
    for (int i = 0; i < d.size(); i++) {
        CHECK(d[i] == d2[i]);
    }
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
