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
#include <hikyuu/trade_manage/crtTM.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_sys/system/imp/WalkForwardTradeManager.h>
#include <hikyuu/trade_manage/margin/crt/MG_ByType.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/signal/crt/SG_Manual.h>
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

/** @par Check point: the walk-forward view answers the margin queries of the account it wraps;
 * before the run window the ratio view reads as no leverage and the quota forbids any opening */
TEST_CASE("test_SYS_WalkForward_margin_view") {
    Stock stk = getStock("sz000001");
    Datetime run_start(201112050000LL);

    TMPtr real = crtTM(Datetime(201101040000LL), 100000.0);
    real->margin(MG_ByType("1:0.20", 0.0, 0.9));
    // A non-default accounting configuration: the view must mirror it or the System pre-checks
    // would size the deposits and the delayed-buy re-size on the view's stale defaults
    real->setParam<int>("precision", 3);
    real->setParam<bool>("margin_accounting", true);
    real->setParam<bool>("support_borrow_cash", true);

    auto view = std::make_shared<WalkForwardTradeManager>(real, run_start);

    /** @arg The model itself is forwarded, not a snapshot taken at construction */
    REQUIRE_UNARY(view->margin() != nullptr);
    CHECK_EQ(view->margin()->name(), "MG_ByType");
    CHECK_EQ(view->getMarginRate(run_start, stk), 0.20);
    real->margin(MG_ByType("1:0.30", 0.0, 0.9));
    CHECK_EQ(view->getMarginRate(run_start, stk), 0.30);

    /** @arg The accounting params of the wrapped account are mirrored on the view */
    CHECK_EQ(view->precision(), 3);
    CHECK_UNARY(view->isMarginAccounted(stk));
    CHECK_UNARY(view->tryGetParam<bool>("margin_accounting", false));
    CHECK_UNARY(view->tryGetParam<bool>("support_borrow_cash", false));

    /** @arg The mirror is read-only: a write through the view never reaches the shared account */
    view->setParam<bool>("support_borrow_cash", false);
    CHECK_UNARY(real->getParam<bool>("support_borrow_cash"));
    view->_mirrorParams();
    CHECK_UNARY(view->tryGetParam<bool>("support_borrow_cash", false));

    /** @arg A cloned view keeps the mirror of its own wrapped account */
    auto cloned = std::dynamic_pointer_cast<WalkForwardTradeManager>(view->clone());
    REQUIRE_UNARY(cloned != nullptr);
    CHECK_EQ(cloned->precision(), 3);
    CHECK_UNARY(cloned->tryGetParam<bool>("support_borrow_cash", false));

    /** @arg Inside the window the quota follows the wrapped account */
    price_t price = 10.0;
    price_t available = 100000.0;
    CHECK_EQ(view->getMaxOpenNumber(run_start, stk, price, available, MarginSide::LONG),
             real->getMaxOpenNumber(run_start, stk, price, available, MarginSide::LONG));
    CHECK_UNARY(view->getMaxOpenNumber(run_start, stk, price, available, MarginSide::LONG) > 0.0);

    /** @arg Before the window nothing may be opened, while the ratio stays a plain 1.0 */
    Datetime before(201111040000LL);
    CHECK_EQ(view->getMarginRate(before, stk), 1.0);
    CHECK_EQ(view->getMaxOpenNumber(before, stk, price, available, MarginSide::LONG), 0.0);

    /** @arg The account level views are forwarded too */
    CHECK_EQ(view->getOccupiedMargin(run_start), real->getOccupiedMargin(run_start));
    CHECK_EQ(view->getAvailableMargin(run_start), real->getAvailableMargin(run_start));
}

/** @par Check point: the margin lock of the view rounds on the mirrored grid of the account it
 * wraps, not on the view's own default precision */
TEST_CASE("test_SYS_WalkForward_margin_view_precision") {
    Stock stk = getStock("sz000001");
    Datetime run_start(201112050000LL);

    TMPtr real = crtTM(Datetime(201101040000LL), 100000.0);
    real->margin(MG_ByType("1:0.112", 0.0, 0.9));
    real->setParam<int>("precision", 3);
    auto view = std::make_shared<WalkForwardTradeManager>(real, run_start);

    // 10.15 × 7 = 71.05 of notional × 0.112 = 7.9576, so the third decimal tells the grids apart:
    // 7.958 on the account grid of 3, 7.96 on the view default grid of 2
    /** @arg The requirement is measured on the mirrored account grid */
    CHECK_EQ(view->getOpenMarginToLock(run_start, stk, 10.15, 7, MarginSide::LONG), 7.958);
    CHECK_EQ(view->getOpenMarginToLock(run_start, stk, 10.15, 7, MarginSide::LONG),
             real->getOpenMarginToLock(run_start, stk, 10.15, 7, MarginSide::LONG));

    /** @arg The mirror is a construction-time snapshot: a later change of the account grid does
     *  not retro-apply to the view, the next window build picks it up instead */
    real->setParam<int>("precision", 2);
    CHECK_EQ(view->getOpenMarginToLock(run_start, stk, 10.15, 7, MarginSide::LONG), 7.958);
    auto refreshed = std::make_shared<WalkForwardTradeManager>(real, run_start);
    CHECK_EQ(refreshed->getOpenMarginToLock(run_start, stk, 10.15, 7, MarginSide::LONG), 7.96);
}

/** @par Check point: a walk-forward run books the margin like the same run over an account that
 * already had the financing, i.e. the host carries the candidate switch onto the real book */
TEST_CASE("test_SYS_WalkForward_margin_run_matches_account_switch") {
    Stock stk = getStock("sz000001");
    KQuery query = KQueryByIndex(-125);
    KData k = stk.getKData(query);
    REQUIRE_UNARY(k.size() >= 10);
    Datetime end = k[k.size() - 1].datetime;

    auto make_tm = [](bool borrow_on_account) {
        auto tm =
          crtTM(Datetime(201101040000LL), 100000.0, TC_Zero(), "SYS", MG_ByType("1:0.5", 0.0, 0.9));
        tm->setParam<bool>("margin_accounting", true);
        tm->setParam<bool>("support_borrow_cash", borrow_on_account);
        return tm;
    };

    // The baseline asks for nothing at the system layer and has the financing on the account; the
    // other one has it only on the system, so the host has to write it onto the real book.
    // Cash borrowing only: the stock-lending key is written symmetrically but has no runner case
    // exercising it end to end yet
    auto base_tm = make_tm(true);
    auto base_sys =
      SYS_WalkForward(SystemList{create_test_sys(3, 5)}, base_tm, 30, 20, SE_MaxFundsOptimal());
    base_sys->setParam<bool>("clean_hold_when_select_changed", false);
    base_sys->run(stk, query);

    auto real_tm = make_tm(false);
    auto candidate = create_test_sys(3, 5);
    candidate->setParam<bool>("support_borrow_cash", true);
    auto sys = SYS_WalkForward(SystemList{candidate}, real_tm, 30, 20, SE_MaxFundsOptimal());
    sys->setParam<bool>("clean_hold_when_select_changed", false);
    sys->run(stk, query);

    /** @arg The switch landed on the shared account the windows trade on */
    CHECK_UNARY(real_tm->getParam<bool>("support_borrow_cash"));

    /** @arg And both runs booked the same trades and the same cash */
    TradeRecordList base_tr = base_tm->getTradeList();
    TradeRecordList tr = real_tm->getTradeList();
    REQUIRE_UNARY(base_tr.size() > 1);
    REQUIRE_EQ(tr.size(), base_tr.size());
    for (size_t i = 0, total = tr.size(); i < total; ++i) {
        CHECK_EQ(tr[i].datetime, base_tr[i].datetime);
        CHECK_EQ(tr[i].business, base_tr[i].business);
        CHECK_EQ(tr[i].number, base_tr[i].number);
        CHECK_EQ(tr[i].realPrice, base_tr[i].realPrice);
    }
    CHECK_EQ(real_tm->currentCash(), base_tm->currentCash());
    auto base_funds = base_tm->getFunds(end);
    auto funds = real_tm->getFunds(end);
    // Pin the margin path as actually exercised over the run, otherwise the parity below turns
    // vacuous once the end position is flat and the end-time occupation reads zero
    DatetimeList trade_dates;
    for (const auto& r : base_tr) {
        trade_dates.push_back(r.datetime);
    }
    FundsList base_funds_curve = base_tm->getFundsList(trade_dates);
    price_t max_occupied = 0.0;
    for (const auto& f : base_funds_curve) {
        if (f.occupied_margin > max_occupied) {
            max_occupied = f.occupied_margin;
        }
    }
    CHECK_UNARY(max_occupied > 0.0);
    CHECK_EQ(funds.net_assets(), base_funds.net_assets());
    CHECK_EQ(funds.borrow_cash, base_funds.borrow_cash);
    CHECK_EQ(funds.occupied_margin, base_funds.occupied_margin);
}

TEST_CASE("test_SYS_WalkForward_runMoment_all_delayed_trades") {
    KData calendar = getStock("sh000001").getKData(KQuery(-8));
    REQUIRE_EQ(calendar.size(), 8);
    KRecordList bars;
    for (size_t i = 0; i < calendar.size(); ++i) {
        bars.emplace_back(calendar[i].datetime, 10, 11, 9, 10, i == 4 ? 0 : 100, 100);
    }
    Stock stock("TEST", "WALK", "Delayed walk forward");
    stock.setKRecordList(bars);
    KData kdata =
      stock.getKData(KQueryByDate(bars.front().datetime, bars.back().datetime + Days(1)));
    // SYS_WalkForward::run() resets the system but keeps the trade manager, so the batch path is
    // verified on a fresh instance instead of re-running this one on an account that is not reset
    auto make_sys = [&kdata]() {
        auto sg = SG_Manual();
        sg->setParam<bool>("alternate", false);
        sg->_addBuySignal(kdata[3].datetime);
        sg->_addBuySignal(kdata[4].datetime);
        auto candidate =
          SYS_Simple(crtTM(), MM_FixedCount(100), EnvironmentPtr(), ConditionPtr(), sg);
        auto sys = SYS_WalkForward(SystemList{candidate},
                                   crtTM(Datetime(199001010000LL), 100000.0, TC_Zero()), 3, 5,
                                   SE_MaxFundsOptimal());
        sys->setTO(kdata);
        sys->readyForRun();
        return sys;
    };

    auto sys = make_sys();
    for (size_t i = 0; i < 5; ++i) {
        sys->runMomentOnOpen(kdata[i].datetime);
        sys->runMomentOnClose(kdata[i].datetime);
    }

    /** @arg the selected candidate forwards both fills to the host result and ledger */
    MomentResult result = sys->runMomentOnOpen(kdata[5].datetime);
    REQUIRE_EQ(result.tradesOnOpen.size(), 2);
    REQUIRE_EQ(sys->getTradeRecordList().size(), 2);
    for (size_t i = 0; i < 2; ++i) {
        CHECK_EQ(result.tradesOnOpen[i], sys->getTradeRecordList()[i]);
        CHECK_EQ(result.tradesOnOpen[i].business, BUSINESS_BUY);
        CHECK_EQ(result.tradesOnOpen[i].number, 100);
        CHECK_EQ(result.tradesOnOpen[i].datetime, kdata[5].datetime);
    }
    CHECK_UNARY(sys->runMomentOnClose(kdata[5].datetime).tradesOnClose.empty());
    CHECK_EQ(sys->getTM()->getHoldNumber(kdata[5].datetime, stock), 200);

    /** @arg an independent batch run contains the same two fills */
    const TradeRecordList moment_trades = sys->getTradeRecordList();
    auto batch = make_sys();
    batch->run(kdata);
    REQUIRE_EQ(batch->getTradeRecordList().size(), moment_trades.size());
    for (size_t i = 0; i < moment_trades.size(); ++i) {
        CHECK_EQ(batch->getTradeRecordList()[i], moment_trades[i]);
    }
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