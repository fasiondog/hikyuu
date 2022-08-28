/*
 * test_export.cpp
 *
 *  Created on: 2013-4-29
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/config.h>

#include <fstream>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/TradeManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_manage/crt/TC_FixedA.h>

using namespace hku;

#if HKU_SUPPORT_SERIALIZATION

/**
 * @defgroup test_trade_manage_export test_trade_manage_export
 * @ingroup test_hikyuu_trade_manage_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_CostRecord_export") {
    string filename(StockManager::instance().tmpdir());
    filename += "/CostRecord.xml";

    CostRecord cost(10.1, 5.3, 1.0, 0.7, 17.1);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(cost);
    }

    CostRecord cost2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(cost2);
    }

    CHECK_EQ(cost, cost2);
    CHECK_EQ(cost.commission, doctest::Approx(cost2.commission).epsilon(0.00001));
    CHECK_EQ(cost.stamptax, doctest::Approx(cost2.stamptax).epsilon(0.00001));
    CHECK_EQ(cost.transferfee, doctest::Approx(cost2.transferfee).epsilon(0.00001));
    CHECK_EQ(cost.others, doctest::Approx(cost2.others).epsilon(0.00001));
    CHECK_EQ(cost.total, doctest::Approx(cost2.total).epsilon(0.00001));
}

/** @par 检测点 */
TEST_CASE("test_TradeRecord_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TradeRecord.xml";

    Stock stock = sm.getStock("sh600000");

    TradeRecord record1;
    record1.stock = stock;
    record1.datetime = Datetime(201305011935);
    record1.business = BUSINESS_BUY;
    record1.planPrice = 10.0;
    record1.realPrice = 10.13;
    record1.goalPrice = 20.15;
    record1.number = 1000;
    record1.stoploss = 9.77;
    record1.cash = 99998.1;
    record1.from = PART_SIGNAL;
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(record1);
    }

    TradeRecord record2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(record2);
    }

    CHECK_EQ(record1, record2);
}

/** @par 检测点 */
TEST_CASE("test_PositionRecord_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/PositionRecord.xml";

    Stock stock = sm.getStock("sh600000");

    PositionRecord record1;
    record1.stock = stock;
    record1.takeDatetime = Datetime(200101010000);
    record1.cleanDatetime = Datetime(200101210000);
    record1.number = 1000;
    record1.stoploss = 10.11;
    record1.goalPrice = 11.13;
    record1.totalNumber = 2000;
    record1.buyMoney = 22000;
    record1.totalCost = 23.1;
    record1.totalRisk = 9.77;
    record1.sellMoney = 9180.1;
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(record1);
    }

    PositionRecord record2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(record2);
    }

    CHECK_EQ(record1, record2);
}

/** @par 检测点 */
TEST_CASE("test_TradeManager_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/TradeManager.xml";

    Stock stock = sm.getStock("sh600000");
    TradeCostPtr costfunc = TC_FixedA(0.0018, 5, 0.001, 0.001, 1.0);

    TradeManagerPtr tm1 = crtTM(Datetime(199901010000), 100000, costfunc);
    tm1->buy(Datetime(199911170000), stock, 27.18, 100, 27.18, 27.18, 0);
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(tm1);
    }

    TradeManagerPtr tm2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(tm2);
    }

    CHECK_EQ(tm1->name(), tm2->name());
    CHECK_EQ(tm1->initCash(), tm2->initCash());
    CHECK_EQ(tm1->initDatetime(), tm2->initDatetime());
    CHECK_EQ(tm1->firstDatetime(), tm2->firstDatetime());
    CHECK_EQ(tm1->lastDatetime(), tm2->lastDatetime());
    CHECK_EQ(tm1->precision(), tm2->precision());

    const TradeRecordList& trade_list_1 = tm1->getTradeList();
    const TradeRecordList& trade_list_2 = tm2->getTradeList();
    CHECK_EQ(trade_list_1.size(), trade_list_2.size());
    size_t total = trade_list_1.size();
    for (size_t i = 0; i < total; i++) {
        CHECK_EQ(trade_list_1[i], trade_list_2[i]);
    }

    PositionRecordList position1 = tm1->getPositionList();
    PositionRecordList position2 = tm2->getPositionList();
    CHECK_EQ(position1.size(), position2.size());
    total = position1.size();
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(position1[i], position2[i]);
    }

    position1 = tm1->getHistoryPositionList();
    position2 = tm2->getHistoryPositionList();
    CHECK_EQ(position1.size(), position2.size());
    total = position1.size();
    for (size_t i = 0; i < total; ++i) {
        CHECK_EQ(position1[i], position2[i]);
    }
}

/** @par 检测点 */
TEST_CASE("test_FundsRecord_export") {
    string filename(StockManager::instance().tmpdir());
    filename += "/FundsRecord.xml";

    FundsRecord funds1;
    funds1.cash = 1.0;
    funds1.market_value = 2.0;
    funds1.base_cash = 3.0;
    funds1.base_asset = 4.0;
    funds1.borrow_cash = 5.0;
    funds1.borrow_asset = 6.0;
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(funds1);
    }

    FundsRecord funds2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(funds2);
    }

    CHECK_EQ(funds1, funds2);
}

/** @par 检测点 */
TEST_CASE("test_BorrowRecord_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/BorrowRecord.xml";

    Stock stock = sm.getStock("sh000001");
    BorrowRecord bor1;
    bor1.stock = stock;
    bor1.number = 2000;
    bor1.value = 22000;
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(bor1);
    }

    BorrowRecord bor2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(bor2);
    }

    CHECK_EQ(bor1.stock, bor2.stock);
    CHECK_EQ(bor1.number, bor2.number);
    CHECK_EQ(bor1.value, bor2.value);
}

/** @} */

#endif /* HKU_SUPPORT_SERIALIZATION */
