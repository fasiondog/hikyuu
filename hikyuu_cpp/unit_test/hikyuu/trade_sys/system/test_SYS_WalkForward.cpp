/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-16
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/trade_sys/system/crt/SYS_WalkForward.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/NOT.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include "create_test_sys.h"

using namespace hku;

/**
 * @defgroup test_SYS_WalkForward test_SYS_WalkForward
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SYS_WalkForword_SE_MaxFundsOptimal_parallel") {
    Stock stk = getStock("sz000001");
    KQuery query = KQueryByIndex(-50);
    TMPtr tm = crtTM();

    /** @arg A single candidate system with SE_MaxFundsOptimal */
    auto se = SE_MaxFundsOptimal();
    se->setParam<bool>("trace", true);
    auto sys = SYS_WalkForward(SystemList{create_test_sys(3, 5)}, tm, 30, 20, se);
    CHECK_EQ(sys->name(), "SYS_WalkForward");
    query = KQueryByIndex(-125);
    sys->setParam<bool>("clean_hold_when_select_changed", false);
    sys->run(stk, query);

    auto delay_request_list = sys->getBuyTradeRequestList();
    CHECK_UNARY(delay_request_list.front().valid);
    CHECK_EQ(delay_request_list.front().business, BUSINESS_BUY);
    CHECK_EQ(delay_request_list.front().datetime, Datetime(20111205));

    tm = sys->getTM();
    CHECK_EQ(tm->currentCash(), 99328.0);

    auto tr_list1 = tm->getTradeList();
    auto tr_list2 = sys->getTradeRecordList();
    CHECK_EQ(tr_list1.size(), tr_list2.size() + 1);
    for (size_t i = 0, total = tr_list2.size(); i < total; i++) {
        CHECK_EQ(tr_list1[i + 1], tr_list2[i]);
    }

    /** @arg Multiple candidate systems with SE_MaxFundsOptimal */
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    SystemList sys_list;
    for (const auto& param : params) {
        sys_list.emplace_back(create_test_sys(param.first, param.second));
    }
    tm->reset();
    REQUIRE(tm->getTradeList().size() == 1);
    sys = SYS_WalkForward(sys_list, tm, 30, 20, se);
    query = KQueryByIndex(-125);
    sys->setParam<bool>("clean_hold_when_select_changed", false);
    sys->run(stk, query);

    delay_request_list = sys->getBuyTradeRequestList();
    CHECK_UNARY(delay_request_list.front().valid);
    CHECK_EQ(delay_request_list.front().business, BUSINESS_BUY);
    CHECK_EQ(delay_request_list.front().datetime, Datetime(20111205));

    tm = sys->getTM();
    CHECK_EQ(tm->currentCash(), 1213.0);

    tr_list1 = tm->getTradeList();
    tr_list2 = sys->getTradeRecordList();
    CHECK_EQ(tr_list1.size(), tr_list2.size() + 1);
    for (size_t i = 0, total = tr_list2.size(); i < total; i++) {
        CHECK_EQ(tr_list1[i + 1], tr_list2[i]);
    }
}

/** @par Test points */
TEST_CASE("test_SYS_WalkForword_SE_PerformanceOptimal_parallel") {
    Stock stk = getStock("sz000001");
    KQuery query = KQueryByIndex(-50);
    TMPtr tm = crtTM();

    /** @arg A single candidate system with SE_MaxFundsOptimal */
    auto se = SE_PerformanceOptimal("Current Total Assets");
    auto sys = SYS_WalkForward(SystemList{create_test_sys(3, 5)}, tm, 30, 20, se);
    CHECK_EQ(sys->name(), "SYS_WalkForward");
    query = KQueryByIndex(-125);
    sys->setParam<bool>("clean_hold_when_select_changed", false);
    sys->run(stk, query);

    auto delay_request_list = sys->getBuyTradeRequestList();
    CHECK_UNARY(delay_request_list.front().valid);
    CHECK_EQ(delay_request_list.front().business, BUSINESS_BUY);
    CHECK_EQ(delay_request_list.front().datetime, Datetime(20111205));

    tm = sys->getTM();
    CHECK_EQ(tm->currentCash(), 99328.0);

    auto tr_list1 = tm->getTradeList();
    auto tr_list2 = sys->getTradeRecordList();
    CHECK_EQ(tr_list1.size(), tr_list2.size() + 1);
    for (size_t i = 0, total = tr_list2.size(); i < total; i++) {
        CHECK_EQ(tr_list1[i + 1], tr_list2[i]);
    }

    /** @arg Multiple candidate systems with SE_MaxFundsOptimal */
    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    SystemList sys_list;
    for (const auto& param : params) {
        sys_list.emplace_back(create_test_sys(param.first, param.second));
    }
    tm->reset();
    REQUIRE(tm->getTradeList().size() == 1);
    sys = SYS_WalkForward(sys_list, tm, 30, 20, se);
    query = KQueryByIndex(-125);
    sys->setParam<bool>("clean_hold_when_select_changed", false);
    sys->run(stk, query);

    delay_request_list = sys->getBuyTradeRequestList();
    CHECK_UNARY(delay_request_list.front().valid);
    CHECK_EQ(delay_request_list.front().business, BUSINESS_BUY);
    CHECK_EQ(delay_request_list.front().datetime, Datetime(20111205));

    tm = sys->getTM();
    CHECK_EQ(tm->currentCash(), 1213.0);

    tr_list1 = tm->getTradeList();
    tr_list2 = sys->getTradeRecordList();
    CHECK_EQ(tr_list1.size(), tr_list2.size() + 1);
    for (size_t i = 0, total = tr_list2.size(); i < total; i++) {
        CHECK_EQ(tr_list1[i + 1], tr_list2[i]);
    }
}

/** @par Check point: runMoment uses the first run window (the window cursor was advanced before
 * use, so window 0 was never set up) and drives the same trades as run() */
TEST_CASE("test_SYS_WalkForward_runMoment_uses_first_window") {
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByIndex(-125);
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 60);

    auto make_sys = []() {
        auto sys =
          SYS_WalkForward(SystemList{create_test_sys(3, 5)}, crtTM(), 30, 20, SE_MaxFundsOptimal());
        sys->setParam<bool>("clean_hold_when_select_changed", false);
        return sys;
    };

    auto sys1 = make_sys();
    sys1->run(stk, query);
    auto bt_trades = sys1->getTradeRecordList();
    REQUIRE(bt_trades.size() > 1);

    auto sys2 = make_sys();
    sys2->setTO(kdata);
    sys2->readyForRun();
    for (size_t i = 0; i < kdata.size(); ++i) {
        sys2->runMoment(kdata[i].datetime);
    }
    auto mo_trades = sys2->getTradeRecordList();

    /** @arg the moment-by-moment driving reproduces the run() trade sequence */
    REQUIRE_EQ(mo_trades.size(), bt_trades.size());
    for (size_t i = 0, total = mo_trades.size(); i < total; ++i) {
        CHECK_EQ(mo_trades[i].datetime, bt_trades[i].datetime);
        CHECK_EQ(mo_trades[i].business, bt_trades[i].business);
        CHECK_EQ(mo_trades[i].number, bt_trades[i].number);
    }
    CHECK_UNARY(mo_trades.empty() || mo_trades[0].datetime < kdata[60].datetime);
}

/** @par Check point: runMomentOnOpen / runMomentOnClose reproduce the run() trade sequence
 * (the intraday driving entry) */
TEST_CASE("test_SYS_WalkForward_runMoment_on_open_close") {
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByIndex(-125);
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 60);

    auto sys =
      SYS_WalkForward(SystemList{create_test_sys(3, 5)}, crtTM(), 30, 20, SE_MaxFundsOptimal());
    sys->setParam<bool>("clean_hold_when_select_changed", false);
    sys->run(stk, query);
    auto bt_trades = sys->getTradeRecordList();
    REQUIRE(bt_trades.size() > 1);

    auto sys2 =
      SYS_WalkForward(SystemList{create_test_sys(3, 5)}, crtTM(), 30, 20, SE_MaxFundsOptimal());
    sys2->setParam<bool>("clean_hold_when_select_changed", false);
    sys2->setTO(kdata);
    sys2->readyForRun();
    for (size_t i = 0; i < kdata.size(); ++i) {
        sys2->runMomentOnOpen(kdata[i].datetime);
        sys2->runMomentOnClose(kdata[i].datetime);
    }
    auto mo_trades = sys2->getTradeRecordList();
    REQUIRE_EQ(mo_trades.size(), bt_trades.size());
    for (size_t i = 0, total = mo_trades.size(); i < total; ++i) {
        CHECK_EQ(mo_trades[i].datetime, bt_trades[i].datetime);
        CHECK_EQ(mo_trades[i].business, bt_trades[i].business);
        CHECK_EQ(mo_trades[i].number, bt_trades[i].number);
    }
}

/** @par Check point: clean_hold_when_select_changed also takes effect on the moment-driven window
 * switches (the backtest-only handling made live and backtest diverge) */
TEST_CASE("test_SYS_WalkForward_runMoment_clean_hold_on_switch") {
    Stock stk = getStock("sz000001");
    REQUIRE(!stk.isNull());
    KQuery query = KQueryByIndex(-125);
    KData kdata = stk.getKData(query);
    REQUIRE(kdata.size() > 60);

    auto drive = [&](bool clean_hold) {
        auto sys =
          SYS_WalkForward(SystemList{create_test_sys(3, 5)}, crtTM(), 30, 20, SE_MaxFundsOptimal());
        sys->setParam<bool>("clean_hold_when_select_changed", clean_hold);
        sys->setTO(kdata);
        sys->readyForRun();
        for (size_t i = 0; i < kdata.size(); ++i) {
            sys->runMomentOnOpen(kdata[i].datetime);
            sys->runMomentOnClose(kdata[i].datetime);
        }
        return sys->getTradeRecordList();
    };

    auto without = drive(false);
    auto with_hold = drive(true);
    REQUIRE(without.size() > 1);
    /** @arg the first divergent trade must be the switch clean-hold sell */
    size_t i = 0;
    while (
      i < with_hold.size() && i < without.size() && with_hold[i].datetime == without[i].datetime &&
      with_hold[i].business == without[i].business && with_hold[i].number == without[i].number) {
        ++i;
    }
    REQUIRE(i < with_hold.size());
    CHECK_EQ(with_hold[i].business, BUSINESS_SELL);
}

/** @par Check point: runMoment without a prior calculation fails loudly instead of trading
 * silently nothing */
TEST_CASE("test_SYS_WalkForward_runMoment_without_calculation_throws") {
    auto sys =
      SYS_WalkForward(SystemList{create_test_sys(3, 5)}, crtTM(), 30, 20, SE_MaxFundsOptimal());
    sys->setStock(getStock("sz000001"));
    CHECK_THROWS_AS(sys->runMoment(Datetime(201112050000LL)), std::exception);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SYS_WalkForword_export") {
    StockManager& sm = StockManager::instance();
    string filename(sm.tmpdir());
    filename += "/SYS_WalkForward.xml";

    vector<std::pair<int, int>> params{{3, 5}, {3, 10}, {5, 10}, {5, 20}};
    SystemList sys_list;
    for (const auto& param : params) {
        sys_list.emplace_back(create_test_sys(param.first, param.second));
    }
    TMPtr tm = crtTM();
    Stock stk = getStock("sz000001");
    auto sys1 = SYS_WalkForward(sys_list, tm, 30, 20);
    auto query = KQueryByIndex(-125);
    sys1->run(stk, query);
    auto tr_list1 = sys1->getTradeRecordList();
    sys1->reset();

    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(sys1);
    }

    SYSPtr sys2;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(sys2);
    }

    CHECK_EQ(sys1->name(), sys2->name());

    sys2->run(stk, query);
    auto tr_list2 = sys2->getTradeRecordList();
    CHECK_EQ(tr_list1.size(), tr_list2.size());
    for (size_t i = 0, total = tr_list2.size(); i < total; i++) {
        CHECK_EQ(tr_list1[i], tr_list2[i]);
    }
}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */