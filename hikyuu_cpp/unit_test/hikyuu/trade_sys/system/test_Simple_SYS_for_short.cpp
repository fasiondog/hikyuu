/*
 * test_Simple_SYS_for_short.cpp
 *
 * Short selling tests of SYS_Simple (ISS-029): the short risk sign in the money manager, the short
 * stop-loss/profit-goal/take-profit exits in the run loop, and the forced covering when EV/CN
 * becomes invalid.
 */

#include "doctest/doctest.h"
#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>
#include <vector>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/condition/ConditionBase.h>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/profitgoal/ProfitGoalBase.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_FixedPercent.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>

using namespace hku;

namespace {

const Datetime OPEN_DATE(199912150000LL);

// A stop-loss/take-profit strategy that supports both directions: the long-side price is below
// the entry and the short-side price above it (reversed for take-profit usage)
class Iss029PercentST : public StoplossBase {
public:
    explicit Iss029PercentST(double p = 0.03, bool reverse = false)
    : StoplossBase("ISS029_PercentST"), m_reverse(reverse) {
        setParam<double>("p", p);
    }

    virtual StoplossPtr _clone() override {
        return make_shared<Iss029PercentST>(getParam<double>("p"), m_reverse);
    }

    virtual price_t getPrice(const Datetime&, price_t price) override {
        double p = getParam<double>("p");
        return roundEx(price * (m_reverse ? 1.0 + p : 1.0 - p), 2);
    }

    virtual price_t getShortPrice(const Datetime&, price_t price) override {
        double p = getParam<double>("p");
        return roundEx(price * (m_reverse ? 1.0 - p : 1.0 + p), 2);
    }

private:
    bool m_reverse{false};
};

// A profit goal supporting shorts; the short target price is based on the average short price
class Iss029PercentPG : public ProfitGoalBase {
public:
    explicit Iss029PercentPG(double p = 0.02) : ProfitGoalBase("ISS029_PercentPG") {
        setParam<double>("p", p);
    }

    virtual ProfitGoalPtr _clone() override {
        return make_shared<Iss029PercentPG>(getParam<double>("p"));
    }

    virtual price_t getGoal(const Datetime&, price_t price) override {
        return roundEx(price * (1.0 + getParam<double>("p")), 2);
    }

    virtual price_t getShortGoal(const Datetime&, price_t price) override {
        Stock stock = getTO().getStock();
        PositionRecord pos = getTM()->getShortPosition(stock);
        // At the moment the short is opened the position does not exist yet, use the planned price
        price_t base = pos.number != 0 ? pos.sellMoney / pos.number : price;
        return roundEx(base * (1.0 - getParam<double>("p")), 2);
    }
};

class Iss029EV : public EnvironmentBase {
public:
    explicit Iss029EV(std::vector<uint64_t> valid = {})
    : EnvironmentBase("ISS029_EV"), m_valid(std::move(valid)) {}

    virtual EnvironmentPtr _clone() override {
        return make_shared<Iss029EV>(m_valid);
    }

    virtual void _calculate() override {
        for (auto d : m_valid) {
            _addValid(Datetime(d));
        }
    }

private:
    std::vector<uint64_t> m_valid;
};

class Iss029CN : public ConditionBase {
public:
    explicit Iss029CN(std::vector<uint64_t> valid = {})
    : ConditionBase("ISS029_CN"), m_valid(std::move(valid)) {}

    virtual ConditionPtr _clone() override {
        return make_shared<Iss029CN>(m_valid);
    }

    virtual void _calculate() override {
        for (auto d : m_valid) {
            _addValid(Datetime(d));
        }
    }

private:
    std::vector<uint64_t> m_valid;
};

// Emits one sell signal at OPEN_DATE so that the system opens a short position there
class Iss029SG : public SignalBase {
public:
    Iss029SG() : SignalBase("ISS029_SG") {}

    virtual SignalPtr _clone() override {
        return make_shared<Iss029SG>();
    }

    virtual void _calculate(const KData& kdata) override {
        if (kdata.getPos(OPEN_DATE) != Null<size_t>()) {
            _addSellSignal(OPEN_DATE);
        }
    }
};

// A take-profit whose short-side price is scripted per date; unlisted dates return 30.0
class Iss029ScriptedTP : public StoplossBase {
public:
    Iss029ScriptedTP() : StoplossBase("ISS029_ScriptedTP") {}

    virtual StoplossPtr _clone() override {
        return make_shared<Iss029ScriptedTP>();
    }

    virtual price_t getPrice(const Datetime&, price_t) override {
        return 0.0;
    }

    virtual price_t getShortPrice(const Datetime& datetime, price_t) override {
        auto iter = m_script.find(datetime);
        return iter != m_script.end() ? iter->second : 30.0;
    }

    void addScript(const Datetime& datetime, price_t price) {
        m_script[datetime] = price;
    }

private:
    std::map<Datetime, price_t> m_script;
};

SignalPtr makeShortSG() {
    SignalPtr sg = make_shared<Iss029SG>();
    sg->setParam<bool>("support_borrow_stock", true);
    return sg;
}

KData getQueryData(const Stock& stk) {
    return stk.getKData(
      KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY));
}

SYSPtr makeShortSys(const SignalPtr& sg, const StoplossPtr& st = StoplossPtr(),
                    const ProfitGoalPtr& pg = ProfitGoalPtr(),
                    const StoplossPtr& tp = StoplossPtr(), const EnvironmentPtr& ev = nullptr,
                    const ConditionPtr& cn = nullptr) {
    SYSPtr sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("support_borrow_stock", true);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(sg);
    sys->setMM(MM_FixedCount(100));
    if (st) {
        sys->setST(st);
    }
    if (pg) {
        sys->setPG(pg);
    }
    if (tp) {
        sys->setTP(tp);
    }
    if (ev) {
        sys->setEV(ev);
    }
    if (cn) {
        sys->setCN(cn);
    }
    return sys;
}

const TradeRecord* findFirstTrade(const TradeRecordList& trs, BUSINESS business) {
    auto iter = std::find_if(trs.begin(), trs.end(),
                             [&](const TradeRecord& tr) { return tr.business == business; });
    return iter == trs.end() ? nullptr : &(*iter);
}

}  // namespace

/**
 * @defgroup test_SYS_Simple_for_short test_SYS_Simple_for_short
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points (the short-side risk guards of the money manager, ISS-029) */
TEST_CASE("test_MM_short_risk_guards") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    Datetime datetime(199912150000LL);

    MMPtr mm = MM_FixedCount(100);
    mm->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));

    /** @arg opening a short: the positive risk is accepted and the fixed number is returned */
    CHECK_EQ(mm->getSellShortNumber(datetime, stk, 26.45, 0.5, PART_SIGNAL), 100.0);

    /** @arg opening a short: a non-positive risk means the stop-loss has been reached, 0 returned
     */
    CHECK_EQ(mm->getSellShortNumber(datetime, stk, 26.45, 0.0, PART_SIGNAL), 0.0);
    CHECK_EQ(mm->getSellShortNumber(datetime, stk, 26.45, -0.5, PART_SIGNAL), 0.0);

    /** @arg covering: when the price reaches the stop-loss, everything is covered (MAX_DOUBLE) */
    CHECK_EQ(mm->getBuyShortNumber(datetime, stk, 27.0, 0.0, PART_STOPLOSS), MAX_DOUBLE);
    CHECK_EQ(mm->getBuyShortNumber(datetime, stk, 27.0, -0.5, PART_STOPLOSS), MAX_DOUBLE);

    /** @arg covering: EV/CN invalidation forces covering the whole short position */
    CHECK_EQ(mm->getBuyShortNumber(datetime, stk, 26.45, 0.5, PART_ENVIRONMENT), MAX_DOUBLE);
    CHECK_EQ(mm->getBuyShortNumber(datetime, stk, 26.45, 0.5, PART_CONDITION), MAX_DOUBLE);
}

/** @par Test points (the default short stop-loss contract, ISS-029) */
TEST_CASE("test_StoplossBase_default_getShortPrice_is_zero") {
    /** @arg a stop-loss without a short-side override returns 0, i.e. no short stop-loss */
    STPtr st = ST_FixedPercent(0.03);
    CHECK_EQ(st->getShortPrice(Datetime(199912150000LL), 26.45), 0.0);
}

/** @par Test points (the short position is covered by the stop-loss, ISS-029) */
TEST_CASE("test_SYS_Simple_short_for_stoploss") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());

    SYSPtr sys = makeShortSys(makeShortSG(), make_shared<Iss029PercentST>(0.03));
    sys->run(stk, src.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* open_tr = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    const TradeRecord* cover_tr = findFirstTrade(tr_list, BUSINESS_BUY_SHORT);
    REQUIRE_NE(open_tr, nullptr);
    REQUIRE_NE(cover_tr, nullptr);

    /** @arg the short is opened at the signal close price with the stop-loss above the entry */
    CHECK_EQ(open_tr->datetime, OPEN_DATE);
    CHECK_LT(std::fabs(open_tr->realPrice - 26.45), 0.00001);
    CHECK_GT(open_tr->number, 0.0);
    CHECK_GT(open_tr->stoploss, open_tr->realPrice);
    CHECK_EQ(open_tr->from, PART_SIGNAL);

    /** @arg the cover is marked as PART_STOPLOSS and happens at the first bar whose close reaches
     *       the stop-loss */
    CHECK_EQ(cover_tr->from, PART_STOPLOSS);
    CHECK_GT(cover_tr->datetime, OPEN_DATE);

    size_t expect_pos = Null<size_t>();
    for (size_t i = open_pos + 1; i < src.size(); ++i) {
        if (src[i].closePrice >= open_tr->stoploss) {
            expect_pos = i;
            break;
        }
    }
    REQUIRE_NE(expect_pos, Null<size_t>());
    CHECK_EQ(cover_tr->datetime, src[expect_pos].datetime);
    CHECK_GE(src[expect_pos].closePrice, open_tr->stoploss);
    CHECK_EQ(cover_tr->number, open_tr->number);
    CHECK_UNARY_FALSE(sys->getTM()->haveShort(stk));
}

/** @par Test points (the short position is covered at the profit goal, ISS-029) */
TEST_CASE("test_SYS_Simple_short_for_profitgoal") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());

    SYSPtr sys = makeShortSys(makeShortSG(), StoplossPtr(), make_shared<Iss029PercentPG>(0.02));
    sys->run(stk, src.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* open_tr = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    const TradeRecord* cover_tr = findFirstTrade(tr_list, BUSINESS_BUY_SHORT);
    REQUIRE_NE(open_tr, nullptr);
    REQUIRE_NE(cover_tr, nullptr);

    /** @arg the goal price stored at the open is below the entry */
    CHECK_LT(open_tr->goalPrice, open_tr->realPrice);

    /** @arg the cover is marked as PART_PROFITGOAL and happens at the first bar whose close falls
     *       to the goal price */
    CHECK_EQ(cover_tr->from, PART_PROFITGOAL);
    CHECK_GT(cover_tr->datetime, OPEN_DATE);

    size_t expect_pos = Null<size_t>();
    for (size_t i = open_pos + 1; i < src.size(); ++i) {
        if (src[i].closePrice <= open_tr->goalPrice) {
            expect_pos = i;
            break;
        }
    }
    REQUIRE_NE(expect_pos, Null<size_t>());
    CHECK_EQ(cover_tr->datetime, src[expect_pos].datetime);
    CHECK_LE(src[expect_pos].closePrice, open_tr->goalPrice);
    CHECK_UNARY_FALSE(sys->getTM()->haveShort(stk));
}

/** @par Test points (the short position is covered by the downward take-profit ratchet, ISS-029) */
TEST_CASE("test_SYS_Simple_short_for_takeprofit") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());

    // A take-profit level 2% below the current price; it trails downward and never rises
    SYSPtr sys = makeShortSys(makeShortSG(), StoplossPtr(), ProfitGoalPtr(),
                              make_shared<Iss029PercentST>(0.02, true));
    sys->run(stk, src.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* open_tr = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    const TradeRecord* cover_tr = findFirstTrade(tr_list, BUSINESS_BUY_SHORT);
    REQUIRE_NE(open_tr, nullptr);
    REQUIRE_NE(cover_tr, nullptr);

    /** @arg the cover is marked as PART_TAKEPROFIT, occurs after the delay and at a profitable
     *       price below the entry */
    CHECK_EQ(cover_tr->from, PART_TAKEPROFIT);
    CHECK_GT(cover_tr->datetime, OPEN_DATE);
    CHECK_LT(cover_tr->realPrice, open_tr->realPrice);
    CHECK_UNARY_FALSE(sys->getTM()->haveShort(stk));
}

/** @par Test points (the short take-profit ratchet keeps the historical lowest level, ISS-029) */
TEST_CASE("test_SYS_Simple_short_for_takeprofit_ratchet") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);

    // With tp_delay_n=3 the scripted take-profit is 25.00 on the two delayed bars (below the
    // price but gated by the delay) and rises to 30.00 from the third bar on. A ratcheting
    // implementation clamps the level to the historical lowest 25.00 and covers on 1999-12-21
    // (close 25.24); a naive implementation following the rising 30.00 never covers
    auto tp = make_shared<Iss029ScriptedTP>();
    tp->addScript(Datetime(199912160000LL), 25.00);
    tp->addScript(Datetime(199912170000LL), 25.00);

    SYSPtr sys = makeShortSys(makeShortSG(), StoplossPtr(), ProfitGoalPtr(), tp);
    sys->setParam<int>("tp_delay_n", 3);
    sys->run(stk, src.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* open_tr = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    const TradeRecord* cover_tr = findFirstTrade(tr_list, BUSINESS_BUY_SHORT);
    REQUIRE_NE(open_tr, nullptr);
    REQUIRE_NE(cover_tr, nullptr);

    /** @arg the delayed bars do not trigger the take-profit although the level is below the price
     */
    CHECK_GT(cover_tr->datetime, Datetime(199912170000LL));

    /** @arg the rising take-profit is clamped to the historical lowest level and covers on
     *       1999-12-21 */
    CHECK_EQ(cover_tr->from, PART_TAKEPROFIT);
    CHECK_EQ(cover_tr->datetime, Datetime(199912210000LL));
    CHECK_LT(std::fabs(cover_tr->realPrice - 25.24), 0.00001);
    CHECK_UNARY_FALSE(sys->getTM()->haveShort(stk));
}

/** @par Test points (the delayed short open stores the short-side stop-loss and quantity, ISS-029)
 */
TEST_CASE("test_SYS_Simple_short_for_delay_open") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());
    REQUIRE(open_pos + 1 < src.size());

    SYSPtr sys = makeShortSys(makeShortSG(), make_shared<Iss029PercentST>(0.03));
    sys->setParam<bool>("buy_delay", true);
    sys->setParam<bool>("sell_delay", true);
    // Use the stop-loss/goal/quantity stored in the pending request
    sys->setParam<bool>("delay_use_current_price", false);
    sys->run(stk, src.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* open_tr = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    REQUIRE_NE(open_tr, nullptr);

    /** @arg the short is opened on the next bar and the stored stop-loss is above the entry */
    CHECK_EQ(open_tr->datetime, src[open_pos + 1].datetime);
    CHECK_GT(open_tr->stoploss, open_tr->realPrice);
    CHECK_EQ(open_tr->number, 100.0);
    CHECK_EQ(open_tr->from, PART_SIGNAL);
}

/** @par Test points (EV invalidation forces covering the short position, ISS-029) */
TEST_CASE("test_SYS_Simple_short_for_ev_force_cover") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);

    EnvironmentPtr ev = make_shared<Iss029EV>(std::vector<uint64_t>{OPEN_DATE.number()});
    SYSPtr sys = makeShortSys(makeShortSG(), StoplossPtr(), ProfitGoalPtr(), StoplossPtr(), ev);
    sys->run(stk, src.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* open_tr = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    const TradeRecord* cover_tr = findFirstTrade(tr_list, BUSINESS_BUY_SHORT);
    REQUIRE_NE(open_tr, nullptr);
    REQUIRE_NE(cover_tr, nullptr);

    /** @arg the cover on the next bar is marked as PART_ENVIRONMENT */
    CHECK_EQ(open_tr->datetime, OPEN_DATE);
    CHECK_EQ(cover_tr->from, PART_ENVIRONMENT);
    CHECK_GT(cover_tr->datetime, OPEN_DATE);
    CHECK_UNARY_FALSE(sys->getTM()->haveShort(stk));
}

/** @par Test points (CN invalidation forces covering the short position, ISS-029) */
TEST_CASE("test_SYS_Simple_short_for_cn_force_cover") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);

    ConditionPtr cn = make_shared<Iss029CN>(std::vector<uint64_t>{OPEN_DATE.number()});
    SYSPtr sys =
      makeShortSys(makeShortSG(), StoplossPtr(), ProfitGoalPtr(), StoplossPtr(), nullptr, cn);
    sys->run(stk, src.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* open_tr = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    const TradeRecord* cover_tr = findFirstTrade(tr_list, BUSINESS_BUY_SHORT);
    REQUIRE_NE(open_tr, nullptr);
    REQUIRE_NE(cover_tr, nullptr);

    /** @arg the cover on the next bar is marked as PART_CONDITION */
    CHECK_EQ(open_tr->datetime, OPEN_DATE);
    CHECK_EQ(cover_tr->from, PART_CONDITION);
    CHECK_GT(cover_tr->datetime, OPEN_DATE);
    CHECK_UNARY_FALSE(sys->getTM()->haveShort(stk));
}

/** @} */
