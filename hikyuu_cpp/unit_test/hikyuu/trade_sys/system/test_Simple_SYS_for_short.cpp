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
#include <tuple>
#include <utility>
#include <vector>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/TC_FixedA2017.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/condition/ConditionBase.h>
#include <hikyuu/trade_sys/environment/EnvironmentBase.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedPercent.h>
#include <hikyuu/trade_sys/profitgoal/ProfitGoalBase.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_FixedPercent.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/slippage/crt/SP_FixedPercent.h>

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

    virtual price_t _getPrice(const Datetime&, price_t price) override {
        double p = getParam<double>("p");
        return roundEx(price * (m_reverse ? 1.0 + p : 1.0 - p), 2);
    }

    virtual price_t _getShortPrice(const Datetime&, price_t price) override {
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

    virtual price_t _getGoal(const Datetime&, price_t price) override {
        return roundEx(price * (1.0 + getParam<double>("p")), 2);
    }

    virtual price_t _getShortGoal(const Datetime&, price_t price) override {
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

// A short-side goal returning the documented sentinels: 0 (no effect on a short) or the current
// price (cover at once)
class Iss029ShortSentinelPG : public ProfitGoalBase {
public:
    explicit Iss029ShortSentinelPG(bool force_close) : ProfitGoalBase("ISS029_ShortSentinelPG") {
        setParam<bool>("force_close", force_close);
    }

    virtual ProfitGoalPtr _clone() override {
        return make_shared<Iss029ShortSentinelPG>(getParam<bool>("force_close"));
    }

    virtual price_t _getGoal(const Datetime&, price_t) override {
        return Null<price_t>();
    }

    virtual price_t _getShortGoal(const Datetime&, price_t price) override {
        return getParam<bool>("force_close") ? price : 0.0;
    }
};

// Exposes the short take-profit ratchet for state assertions
class RatchetProbeSYS : public System {
public:
    RatchetProbeSYS() : System("RatchetProbeSYS") {}

    price_t shortRatchet() const {
        return m_lastShortTakeProfit;
    }

    void setShortRatchet(price_t value) {
        m_lastShortTakeProfit = value;
    }
};

// A take-profit whose short-side price is scripted per date; unlisted dates return the default
class Iss029ScriptedTP : public StoplossBase {
public:
    explicit Iss029ScriptedTP(price_t defaultPrice = 30.0)
    : StoplossBase("ISS029_ScriptedTP"), m_defaultPrice(defaultPrice) {}

    virtual StoplossPtr _clone() override {
        return make_shared<Iss029ScriptedTP>(m_defaultPrice);
    }

    virtual price_t _getPrice(const Datetime&, price_t) override {
        return 0.0;
    }

    virtual price_t _getShortPrice(const Datetime& datetime, price_t) override {
        auto iter = m_script.find(datetime);
        return iter != m_script.end() ? iter->second : m_defaultPrice;
    }

    void addScript(const Datetime& datetime, price_t price) {
        m_script[datetime] = price;
    }

private:
    price_t m_defaultPrice;
    std::map<Datetime, price_t> m_script;
};

// A money manager opening 200 shares short once and covering 100 per exit, so a TP cover is partial
class TpCoverPartialMM : public MoneyManagerBase {
public:
    TpCoverPartialMM() : MoneyManagerBase("TP_COVER_PARTIAL_MM") {}

    virtual MoneyManagerPtr _clone() override {
        return make_shared<TpCoverPartialMM>();
    }

    // The pure-virtual long-side hook is never reached: the system only shorts
    virtual double _getBuyNumber(const Datetime&, const Stock&, price_t, price_t,
                                 SystemPart) override {
        return 0.0;
    }

    virtual double _getSellShortNumber(const Datetime&, const Stock&, price_t, price_t,
                                       SystemPart) override {
        return 200.0;
    }

    virtual double _getBuyShortNumber(const Datetime&, const Stock&, price_t, price_t,
                                      SystemPart) override {
        return 100.0;
    }
};

SignalPtr makeShortSG() {
    SignalPtr sg = make_shared<Iss029SG>();
    sg->setParam<bool>("support_borrow_stock", true);
    return sg;
}

// A scripted short flow: the sell signal opens the short on the open date, the buy signal covers
// it on the cover date; a Null cover date leaves the short open until a forced cover
class ShortOpenCoverSG : public SignalBase {
public:
    explicit ShortOpenCoverSG(Datetime open_date, Datetime cover_date = Null<Datetime>())
    : SignalBase("ShortOpenCoverSG"), m_open_date(open_date), m_cover_date(cover_date) {}

    virtual SignalPtr _clone() override {
        return make_shared<ShortOpenCoverSG>(m_open_date, m_cover_date);
    }

    virtual void _calculate(const KData& kdata) override {
        if (kdata.getPos(m_open_date) != Null<size_t>()) {
            _addSellSignal(m_open_date);
        }
        if (!m_cover_date.isNull() && kdata.getPos(m_cover_date) != Null<size_t>()) {
            _addBuySignal(m_cover_date);
        }
    }

private:
    Datetime m_open_date;
    Datetime m_cover_date;
};

// The probe system with the short-side parts set up for the partial cover cases
std::shared_ptr<RatchetProbeSYS> makeShortRatchetProbeSys(const KData& k, const StoplossPtr& tp,
                                                          bool buyDelay) {
    auto sys = std::make_shared<RatchetProbeSYS>();
    sys->setParam<bool>("buy_delay", buyDelay);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("support_borrow_stock", true);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(makeShortSG());
    sys->setMM(make_shared<TpCoverPartialMM>());
    sys->setTP(tp);
    sys->setTO(k);
    sys->readyForRun();
    return sys;
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

// The short covers of the trade list, ignoring the borrow / return stock records
std::vector<const TradeRecord*> shortCovers(const TradeRecordList& trs) {
    std::vector<const TradeRecord*> covers;
    for (const auto& tr : trs) {
        if (tr.business == BUSINESS_BUY_SHORT) {
            covers.push_back(&tr);
        }
    }
    return covers;
}

// Mirror System::fixupSlippagePrice: round to the market price precision, then clamp into the bar
price_t expectSlipped(price_t plan, double factor, const KRecord& rec, const Stock& stk) {
    price_t p = roundEx(plan * factor, stk.precision());
    return std::min(std::max(p, rec.lowPrice), rec.highPrice);
}

// Emits scripted long/short signals on one account: buy -> sell (close the long) -> sell (open
// the short) -> buy (cover the short)
class Iss029MixedSG : public SignalBase {
public:
    Iss029MixedSG() : SignalBase("ISS029_MixedSG") {
        setParam<bool>("support_borrow_stock", true);
    }

    virtual SignalPtr _clone() override {
        return make_shared<Iss029MixedSG>();
    }

    virtual void _calculate(const KData& kdata) override {
        _addBuySignal(Datetime(199912150000LL));
        _addSellSignal(Datetime(199912220000LL));
        _addSellSignal(Datetime(200001050000LL));
        _addBuySignal(Datetime(200001060000LL));
    }
};

// Emits one buy signal on the given date, so the system opens a long position
class ScriptedBuySG : public SignalBase {
public:
    explicit ScriptedBuySG(Datetime buy_date) : SignalBase("ScriptedBuySG"), m_buy_date(buy_date) {}

    virtual SignalPtr _clone() override {
        return make_shared<ScriptedBuySG>(m_buy_date);
    }

    virtual void _calculate(const KData& kdata) override {
        if (kdata.getPos(m_buy_date) != Null<size_t>()) {
            _addBuySignal(m_buy_date);
        }
    }

private:
    Datetime m_buy_date;
};

// The cash-gap short scenario: 13.58 -> 23.63 between the open and the cover is a 74% rise, more
// than the 1 / 60% margin allowed at the short opening, so the proceeds cannot pay the buyback
const Datetime SHORT_GAP_OPEN(200611060000LL);
const Datetime SHORT_GAP_COVER(200701100000LL);
const Datetime SHORT_GAP_COVER_FILL(200701110000LL);
const double SHORT_GAP_NUM = 1000.0;

KData getShortGapKData(const Stock& stk) {
    return stk.getKData(
      KQueryByDate(Datetime(200610010000LL), Datetime(200703010000LL), KQuery::DAY));
}

// 9000 cash: 13.58 x 1000 x 60% = 8148 passes the opening margin, while the 23.63 buyback of
// 23630 exceeds the 22580 cash left after the short
SYSPtr makeShortGapSys(bool auto_checkin, bool buy_delay, const SignalPtr& sg) {
    SYSPtr sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", buy_delay);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("support_borrow_stock", true);
    sys->setTM(crtTM(Datetime(199001010000LL), 9000, TC_Zero(), "TEST_TM"));
    // The short opening needs a stop-loss above the entry; 190% keeps it above every close of the
    // holding window so it never covers before the end
    sys->setST(make_shared<Iss029PercentST>(1.9));
    sg->setParam<bool>("support_borrow_stock", true);
    sys->setSG(sg);
    MMPtr mm = MM_FixedCount(SHORT_GAP_NUM);
    mm->setParam<bool>("auto_checkin", auto_checkin);
    sys->setMM(mm);
    return sys;
}

struct ShortCaseResult {
    TradeRecordList trades;
    double short_left{0.0};
};

ShortCaseResult collectShortCase(const SYSPtr& sys, const Stock& stk) {
    ShortCaseResult result;
    result.trades = sys->getTM()->getTradeList();
    result.short_left = sys->getTM()->getShortPosition(stk).number;
    return result;
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

// A long-only stop-loss: it does not override _getShortPrice, so it must fall back to the base
// default (0 = no short stop-loss), per StoplossBase's documented contract.
class Iss029LongOnlyST : public StoplossBase {
public:
    Iss029LongOnlyST() : StoplossBase("ISS029_LongOnlyST") {}
    virtual StoplossPtr _clone() override {
        return make_shared<Iss029LongOnlyST>();
    }
    virtual price_t _getPrice(const Datetime&, price_t price) override {
        return price * 0.9;
    }
};

/** @par Test points (the default short stop-loss contract, ISS-029) */
TEST_CASE("test_StoplossBase_default_getShortPrice_is_zero") {
    /** @arg a stop-loss without a short-side override returns 0, i.e. no short stop-loss */
    STPtr st = make_shared<Iss029LongOnlyST>();
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

/** @par Test points (a forced flatten clears the short take-profit ratchet once the book is flat)
 */
TEST_CASE("test_System_sellForce_resets_short_tp_ratchet") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());

    auto sys = std::make_shared<RatchetProbeSYS>();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("support_borrow_stock", true);
    sys->setParam<int>("tp_delay_n", 1);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(makeShortSG());
    sys->setMM(MM_FixedCount(100));
    sys->setTO(src);
    sys->readyForRun();

    sys->runMoment(OPEN_DATE);
    REQUIRE_UNARY(sys->getTM()->haveShort(stk));
    CHECK_GT(sys->shortRatchet(), 0.0);

    TradeRecord tr = sys->sellForceOnClose(src[open_pos + 1].datetime, MAX_DOUBLE, PART_PORTFOLIO);
    REQUIRE_EQ(tr.business, BUSINESS_BUY_SHORT);
    CHECK_UNARY_FALSE(sys->getTM()->haveShort(stk));

    /** @arg the ratchet is cleared together with the covered short book */
    CHECK_EQ(sys->shortRatchet(), 0.0);
}

/** @par Test points (a zero short ratchet marks a flat book and must not clamp the take-profit
 *  level down to zero) */
TEST_CASE("test_System_short_tp_zero_ratchet_does_not_clamp") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());
    const Datetime next = src[open_pos + 1].datetime;

    auto tp = make_shared<Iss029ScriptedTP>();
    tp->addScript(next, 30.0);  // above the price, so that bar has no take-profit

    auto sys = std::make_shared<RatchetProbeSYS>();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setParam<bool>("support_borrow_stock", true);
    sys->setParam<int>("tp_delay_n", 1);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(makeShortSG());
    sys->setMM(MM_FixedCount(100));
    sys->setTP(tp);
    sys->setTO(src);
    sys->readyForRun();

    sys->runMoment(OPEN_DATE);
    REQUIRE_UNARY(sys->getTM()->haveShort(stk));

    // A ratchet of zero while the book is open (a reset that kept the account, e.g. shared_tm)
    sys->setShortRatchet(0.0);
    sys->runMoment(next);

    /** @arg the scripted level is taken as is instead of being clamped to zero */
    CHECK_UNARY(sys->getTM()->haveShort(stk));
    CHECK_EQ(sys->shortRatchet(), 30.0);
}

/** @par Test points (the short-side goal sentinels: 0 never covers, the current price covers at
 * once) */
TEST_CASE("test_SYS_Simple_short_profitgoal_sentinels") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());

    // Returns: whether a PART_PROFITGOAL cover happened, its date, and whether the short is open
    auto run_with = [&](bool force_close) {
        SYSPtr sys = makeShortSys(makeShortSG(), StoplossPtr(),
                                  make_shared<Iss029ShortSentinelPG>(force_close));
        sys->run(stk, src.getQuery());
        Datetime cover_date;
        for (const auto& tr : sys->getTM()->getTradeList()) {
            if (tr.business == BUSINESS_BUY_SHORT && tr.from == PART_PROFITGOAL) {
                cover_date = tr.datetime;
                break;
            }
        }
        return std::make_tuple(!cover_date.isNull(), cover_date, sys->getTM()->haveShort(stk));
    };

    /** @arg returning 0 has no effect on the short side, so the position is never covered */
    auto zero = run_with(false);
    CHECK_UNARY_FALSE(std::get<0>(zero));
    CHECK_UNARY(std::get<2>(zero));

    /** @arg returning the current price covers the short on the bar after the open */
    auto forced = run_with(true);
    CHECK_UNARY(std::get<0>(forced));
    CHECK_EQ(std::get<1>(forced), src[open_pos + 1].datetime);
    CHECK_UNARY_FALSE(std::get<2>(forced));
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

/** @par Test points (the short take-profit is net of the fees, on the same basis as the long side)
 */
TEST_CASE("test_SYS_Simple_short_takeprofit_net_of_fees") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    KData src = getQueryData(stk);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());

    // A level far below the market, so only the profit condition gates the cover
    auto tp = make_shared<Iss029ScriptedTP>();
    tp->addScript(src[open_pos + 1].datetime, 1.0);
    tp->addScript(src[open_pos + 2].datetime, 1.0);

    SYSPtr sys = makeShortSys(makeShortSG(), StoplossPtr(), ProfitGoalPtr(), tp);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_FixedA2017(0.03, 5.0), "TEST_TM"));
    sys->setParam<int>("tp_delay_n", 1);
    sys->run(stk, src.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* open_tr = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    const TradeRecord* cover_tr = findFirstTrade(tr_list, BUSINESS_BUY_SHORT);
    REQUIRE_NE(open_tr, nullptr);
    REQUIRE_NE(cover_tr, nullptr);

    const price_t fee = open_tr->cost.total;
    size_t gross_pos = Null<size_t>();
    size_t net_pos = Null<size_t>();
    for (size_t i = open_pos + 1; i < src.size(); i++) {
        price_t gross = open_tr->number * stk.unit() * (open_tr->realPrice - src[i].closePrice);
        if (gross_pos == Null<size_t>() && gross > 0.0) {
            gross_pos = i;
        }
        if (net_pos == Null<size_t>() && gross - fee > 0.0) {
            net_pos = i;
        }
    }
    REQUIRE_NE(gross_pos, Null<size_t>());
    REQUIRE_NE(net_pos, Null<size_t>());

    /** @arg the fee window exists: the gross profit turns positive before the net one */
    CHECK_UNARY(gross_pos != net_pos);

    /** @arg the cover waits until the profit is positive after the fees */
    CHECK_EQ(cover_tr->from, PART_TAKEPROFIT);
    CHECK_EQ(cover_tr->datetime, src[net_pos].datetime);
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

/** @par Test points (one account interleaving the long and the short sides) */
TEST_CASE("test_SYS_Simple_mixed_long_short") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k = getQueryData(stock);
    REQUIRE(k.size() > 0);

    auto sys = makeShortSys(make_shared<Iss029MixedSG>());
    auto tm = sys->getTM();
    tm->setParam<bool>("support_borrow_stock", true);
    sys->setMM(MM_FixedPercent(0.03));

    sys->run(k);

    TradeRecordList tr_list = tm->getTradeList();

    /** @arg the long buy is sized by the MM and aligned to the lot size: 100000 * 0.03 / 26.45
     * = 113.4 -> 100 shares */
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_EQ(tr_list[1].datetime, Datetime(199912150000LL));
    CHECK_EQ(tr_list[1].number, 100.0);
    CHECK_EQ(tr_list[1].realPrice, 26.45);

    /** @arg the long sell liquidates the entire position */
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_EQ(tr_list[2].datetime, Datetime(199912220000LL));
    CHECK_EQ(tr_list[2].number, 100.0);

    /** @arg the short entry uses the same formula as the long buy and is aligned to the lot
     * size: 99870 * 0.03 / 25.28 = 118.5 -> 100 shares, with the borrowed stock recorded */
    CHECK_EQ(tr_list[3].business, BUSINESS_BORROW_STOCK);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[3].number, 100.0);
    CHECK_EQ(tr_list[4].business, BUSINESS_SELL_SHORT);
    CHECK_EQ(tr_list[4].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[4].number, 100.0);
    CHECK_EQ(tr_list[4].realPrice, 25.28);

    /** @arg the short cover liquidates the entire position and returns the borrowed stock */
    CHECK_EQ(tr_list[5].business, BUSINESS_BUY_SHORT);
    CHECK_EQ(tr_list[5].datetime, Datetime(200001060000LL));
    CHECK_EQ(tr_list[5].number, 100.0);
    CHECK_EQ(tr_list[6].business, BUSINESS_RETURN_STOCK);
    CHECK_EQ(tr_list[6].number, 100.0);

    /** @arg no other trades: exactly one long round trip and one short round trip */
    CHECK_EQ(tr_list.size(), 7);
}

/** @par Test points (the short-side slippage hooks are used by the short trades) */
TEST_CASE("test_SYS_Simple_short_for_slippage") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k = getQueryData(stock);
    REQUIRE(k.size() > 0);

    auto sys = makeShortSys(make_shared<Iss029MixedSG>());
    auto tm = sys->getTM();
    tm->setParam<bool>("support_borrow_stock", true);
    sys->setMM(MM_FixedPercent(0.03));
    sys->setSP(SP_FixedPercent(0.001, 0.02));
    sys->run(k);

    const TradeRecordList& tr_list = tm->getTradeList();
    const TradeRecord* long_buy = findFirstTrade(tr_list, BUSINESS_BUY);
    const TradeRecord* short_open = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    const TradeRecord* short_cover = findFirstTrade(tr_list, BUSINESS_BUY_SHORT);
    REQUIRE_NE(long_buy, nullptr);
    REQUIRE_NE(short_open, nullptr);
    REQUIRE_NE(short_cover, nullptr);

    /** @arg the long buy slips by p only, untouched by short_p */
    CHECK(long_buy->realPrice ==
          doctest::Approx(
            expectSlipped(long_buy->planPrice, 1.001, k[k.getPos(long_buy->datetime)], stock)));

    /** @arg opening a short slips by short_p in the sell direction */
    CHECK(short_open->realPrice < short_open->planPrice);
    CHECK(short_open->realPrice ==
          doctest::Approx(
            expectSlipped(short_open->planPrice, 0.98, k[k.getPos(short_open->datetime)], stock)));

    /** @arg covering a short slips by short_p in the buy direction */
    CHECK(short_cover->realPrice > short_cover->planPrice);
    CHECK(short_cover->realPrice ==
          doctest::Approx(expectSlipped(short_cover->planPrice, 1.02,
                                        k[k.getPos(short_cover->datetime)], stock)));

    /** @arg the trade structure is unaffected by the slippage */
    CHECK_EQ(tr_list.size(), 7);
}

/** @par Test points (the delayed short paths take the short-side slippage hooks too) */
TEST_CASE("test_SYS_Simple_short_for_slippage_delay") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh600000"];
    KData src = getQueryData(stock);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());
    REQUIRE(open_pos + 1 < src.size());

    SYSPtr sys = makeShortSys(makeShortSG(), make_shared<Iss029PercentST>(0.03));
    sys->setSP(SP_FixedPercent(0.001, 0.02));
    sys->setParam<bool>("buy_delay", true);
    sys->setParam<bool>("sell_delay", true);
    sys->setParam<bool>("delay_use_current_price", false);
    sys->run(stock, src.getQuery());

    const TradeRecordList& tr_list = sys->getTM()->getTradeList();
    const TradeRecord* short_open = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    const TradeRecord* short_cover = findFirstTrade(tr_list, BUSINESS_BUY_SHORT);
    REQUIRE_NE(short_open, nullptr);
    REQUIRE_NE(short_cover, nullptr);

    /** @arg the delayed short open still takes the short-side sell price */
    CHECK_EQ(short_open->datetime, src[open_pos + 1].datetime);
    CHECK(short_open->realPrice ==
          doctest::Approx(expectSlipped(short_open->planPrice, 0.98,
                                        src[src.getPos(short_open->datetime)], stock)));

    /** @arg the delayed cover still takes the short-side buy price */
    CHECK(short_cover->realPrice ==
          doctest::Approx(expectSlipped(short_cover->planPrice, 1.02,
                                        src[src.getPos(short_cover->datetime)], stock)));
}

/** @par Test points (a forced cover uses the short-side buy price, not the sell price) */
TEST_CASE("test_SYS_Simple_short_for_force_cover_slippage") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh600000"];
    KData src = getQueryData(stock);
    size_t open_pos = src.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());
    REQUIRE(open_pos + 1 < src.size());

    // No stop-loss / goal, so the short stays open at the end of the run
    SYSPtr sys = makeShortSys(makeShortSG());
    sys->setSP(SP_FixedPercent(0.001, 0.02));
    sys->run(stock, src.getQuery());
    REQUIRE_UNARY(sys->getTM()->haveShort(stock));

    const KRecord rec = src[open_pos + 1];
    TradeRecord force_tr = sys->sellForceOnClose(rec.datetime, MAX_DOUBLE, PART_PORTFOLIO);

    /** @arg the forced flatten covers the short at the short-side buy price (above the plan) */
    REQUIRE_EQ(force_tr.business, BUSINESS_BUY_SHORT);
    CHECK(force_tr.realPrice >= rec.closePrice);
    CHECK(force_tr.realPrice == doctest::Approx(expectSlipped(rec.closePrice, 1.02, rec, stock)));
    CHECK_UNARY_FALSE(sys->getTM()->haveShort(stock));
}

//-----------------------------------------------------------------------------
// The short take-profit ratchet on a partial cover: a partial cover keeps the
// low-water mark; only a flat short book resets the ratchet to 0
//-----------------------------------------------------------------------------

/** @par Check points (the short take-profit ratchet on a partial cover):
 *  the first partial cover keeps the ratchet at its level instead of resetting it to the fill, so
 *  a later mild fall above that level still covers the second tranche */
TEST_CASE("test_SYS_Simple_short_tp_partial_cover_keeps_ratchet") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k = getQueryData(stock);
    REQUIRE(k.size() > 10);
    size_t open_pos = k.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());

    // t1: the first tradable fall under the open close; the level l1 = close[t1] * 0.94 covers
    // there and lowers the ratchet to l1. t2 (> t1): a mild further fall, close[t1] * 0.96 <
    // close[t2] <= close[t1] * 0.98; the level l2 = close[t1] * 0.999 sits between the t1 fill
    // and the ratchet l1, so with the kept ratchet the fall covers the second tranche, while a
    // reset-to-fill ratchet (min(l2, fill) = l2) stays silent. Both outcomes keep a margin of
    // about 3% to the level that decides them
    size_t t1 = 0;
    size_t t2 = 0;
    for (size_t i = open_pos + 1; i + 1 < k.size() && t2 == 0; ++i) {
        const KRecord& r1 = k[i];
        if (r1.closePrice >= k[open_pos].closePrice || r1.highPrice == r1.lowPrice ||
            r1.transAmount == 0.0) {
            continue;
        }
        for (size_t j = i + 1; j < k.size(); ++j) {
            const KRecord& r2 = k[j];
            if (r2.highPrice == r2.lowPrice || r2.transAmount == 0.0) {
                continue;
            }
            if (r2.closePrice < r1.closePrice * 0.98 && r2.closePrice > r1.closePrice * 0.96) {
                t1 = i;
                t2 = j;
                break;
            }
        }
    }
    REQUIRE_UNARY(t2 != 0);

    const price_t l1 = k[t1].closePrice * 0.94;
    auto tp = make_shared<Iss029ScriptedTP>(0.0);
    tp->addScript(k[t1].datetime, l1);
    tp->addScript(k[t2].datetime, k[t1].closePrice * 0.999);

    auto sys = makeShortRatchetProbeSys(k, tp, false);
    for (size_t i = 0; i <= t1; ++i) {
        sys->runMoment(k[i].datetime);
    }

    // The borrow records share this list, so the trades are filtered by business type
    TradeRecordList tr_list = sys->getTM()->getTradeList();
    const TradeRecord* open_tr = findFirstTrade(tr_list, BUSINESS_SELL_SHORT);
    REQUIRE_UNARY(open_tr != nullptr);
    CHECK_EQ(open_tr->number, 200.0);

    std::vector<const TradeRecord*> covers = shortCovers(tr_list);
    REQUIRE_EQ(covers.size(), 1);
    /** @arg the first partial cover fires on the l1 bar */
    CHECK_EQ(covers[0]->datetime, k[t1].datetime);
    CHECK_EQ(covers[0]->number, 100.0);
    CHECK_EQ(covers[0]->from, PART_TAKEPROFIT);

    /** @arg the ratchet after the partial cover is the level that fired, not the fill price */
    CHECK_LT(std::fabs(sys->shortRatchet() - l1), l1 * 0.005);

    for (size_t i = t1 + 1; i <= t2; ++i) {
        sys->runMoment(k[i].datetime);
    }

    tr_list = sys->getTM()->getTradeList();
    covers = shortCovers(tr_list);
    REQUIRE_EQ(covers.size(), 2);
    /** @arg the second partial cover still fires on the fall above the kept ratchet */
    CHECK_EQ(covers[1]->datetime, k[t2].datetime);
    CHECK_EQ(covers[1]->number, 100.0);
    CHECK_EQ(covers[1]->from, PART_TAKEPROFIT);
    CHECK_UNARY_FALSE(sys->getTM()->haveShort(stock));

    /** @arg a flat short book resets the ratchet to 0 */
    CHECK_EQ(sys->shortRatchet(), 0.0);
}

/** @par Check points (a delayed partial cover keeps the ratchet):
 *  the cover request submitted on the take-profit bar is filled at the next open, which must
 * neither cover the whole position nor reset the ratchet to the open price */
TEST_CASE("test_SYS_Simple_short_tp_partial_cover_keeps_ratchet_when_delayed") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData k = getQueryData(stock);
    REQUIRE(k.size() > 10);
    size_t open_pos = k.getPos(OPEN_DATE);
    REQUIRE_NE(open_pos, Null<size_t>());

    // The first tradable fall under the open close, so the short profit gate is met
    size_t t1 = 0;
    for (size_t i = open_pos + 1; i + 1 < k.size(); ++i) {
        const KRecord& r = k[i];
        if (r.closePrice < k[open_pos].closePrice && r.highPrice != r.lowPrice &&
            r.transAmount != 0.0) {
            t1 = i;
            break;
        }
    }
    REQUIRE_UNARY(t1 != 0);

    const price_t l1 = k[t1].closePrice * 0.94;
    auto tp = make_shared<Iss029ScriptedTP>(0.0);
    tp->addScript(k[t1].datetime, l1);

    auto sys = makeShortRatchetProbeSys(k, tp, true);  // the covers are delayed to the next open
    for (size_t i = 0; i <= t1; ++i) {
        sys->runMoment(k[i].datetime);
    }

    TradeRecordList tr_list = sys->getTM()->getTradeList();
    REQUIRE_EQ(shortCovers(tr_list).size(), 0);  // the cover is still only a pending request
    CHECK_UNARY(sys->getTM()->haveShort(stock));
    CHECK_LT(std::fabs(sys->shortRatchet() - l1), l1 * 0.005);

    sys->runMoment(k[t1 + 1].datetime);

    tr_list = sys->getTM()->getTradeList();
    std::vector<const TradeRecord*> covers = shortCovers(tr_list);
    REQUIRE_EQ(covers.size(), 1);
    CHECK_EQ(covers[0]->datetime, k[t1 + 1].datetime);
    CHECK_EQ(covers[0]->number, 100.0);
    CHECK_EQ(covers[0]->from, PART_TAKEPROFIT);

    /** @arg the delayed partial cover covers a tranche instead of the whole short position */
    CHECK_UNARY(sys->getTM()->haveShort(stock));

    /** @arg the delayed partial fill keeps the ratchet instead of resetting it to the open price */
    CHECK_LT(std::fabs(sys->shortRatchet() - l1), l1 * 0.005);
}

/** @par Test point (the auto_checkin also funds the short cover: the buyback needs cash like the
 * long buy) */
TEST_CASE("test_SYS_Simple_short_auto_checkin_cover") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    REQUIRE_UNARY(!stk.isNull());
    KData k = getShortGapKData(stk);
    REQUIRE_UNARY(k.getPos(SHORT_GAP_OPEN) != Null<size_t>());
    REQUIRE_UNARY(k.getPos(SHORT_GAP_COVER) != Null<size_t>());
    REQUIRE_UNARY(k.getPos(SHORT_GAP_COVER_FILL) != Null<size_t>());

    auto run_case = [&](bool auto_checkin, bool buy_delay) {
        SYSPtr sys = makeShortGapSys(
          auto_checkin, buy_delay, make_shared<ShortOpenCoverSG>(SHORT_GAP_OPEN, SHORT_GAP_COVER));
        sys->run(k);
        return collectShortCase(sys, stk);
    };

    for (bool buy_delay : {false, true}) {
        // The immediate cover fills on the signal close, the delayed one on the next open: both
        // are beyond the cash left after the short
        const Datetime fill_date = buy_delay ? SHORT_GAP_COVER_FILL : SHORT_GAP_COVER;

        /** @arg without the top-up the buyback is rejected for lack of cash: the short stays open
         */
        const ShortCaseResult no_top_up = run_case(false, buy_delay);
        CHECK_UNARY(findFirstTrade(no_top_up.trades, BUSINESS_SELL_SHORT) != nullptr);
        CHECK_UNARY(findFirstTrade(no_top_up.trades, BUSINESS_BUY_SHORT) == nullptr);
        CHECK_EQ(no_top_up.short_left, SHORT_GAP_NUM);

        /** @arg with the top-up the cover fills in full on the cash-poor account */
        const ShortCaseResult topped = run_case(true, buy_delay);
        const TradeRecord* open_tr = findFirstTrade(topped.trades, BUSINESS_SELL_SHORT);
        const TradeRecord* cover_tr = findFirstTrade(topped.trades, BUSINESS_BUY_SHORT);
        REQUIRE_NE(open_tr, nullptr);
        REQUIRE_NE(cover_tr, nullptr);
        CHECK_EQ(cover_tr->datetime, fill_date);
        CHECK_EQ(cover_tr->number, open_tr->number);
        CHECK_EQ(topped.short_left, 0.0);

        /** @arg the scenario was really cash-poor: the buyback exceeds the cash after the short */
        const price_t cover_money = cover_tr->realPrice * cover_tr->number * stk.unit();
        CHECK_GT(cover_money, open_tr->cash);

        /** @arg the deposit happens at the fill bar and covers exactly the gap */
        const TradeRecord* checkin_tr = nullptr;
        for (const auto& tr : topped.trades) {
            if (tr.business == BUSINESS_CHECKIN) {
                checkin_tr = &tr;
            }
        }
        REQUIRE_NE(checkin_tr, nullptr);
        CHECK_EQ(checkin_tr->datetime, fill_date);
        CHECK_LT(std::fabs(checkin_tr->realPrice - (cover_money - open_tr->cash)), 0.01);
    }
}

/** @par Test point (the forced cover of a short gets the same top-up: a liquidation must not be
 * blocked on a cash-poor account) */
TEST_CASE("test_SYS_Simple_short_auto_checkin_forced_cover") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    REQUIRE_UNARY(!stk.isNull());
    KData k = getShortGapKData(stk);
    REQUIRE_UNARY(k.getPos(SHORT_GAP_OPEN) != Null<size_t>());
    REQUIRE_UNARY(k.getPos(SHORT_GAP_COVER) != Null<size_t>());

    // No cover signal: the short stays open until the portfolio forces it
    auto run_case = [&](bool auto_checkin) {
        SYSPtr sys =
          makeShortGapSys(auto_checkin, false, make_shared<ShortOpenCoverSG>(SHORT_GAP_OPEN));
        sys->run(k);
        sys->sellForceOnClose(SHORT_GAP_COVER, MAX_DOUBLE, PART_PORTFOLIO);
        return collectShortCase(sys, stk);
    };

    /** @arg without the top-up the forced cover is rejected for lack of cash: the short stays open
     */
    const ShortCaseResult no_top_up = run_case(false);
    CHECK_UNARY(findFirstTrade(no_top_up.trades, BUSINESS_SELL_SHORT) != nullptr);
    CHECK_UNARY(findFirstTrade(no_top_up.trades, BUSINESS_BUY_SHORT) == nullptr);
    CHECK_EQ(no_top_up.short_left, SHORT_GAP_NUM);

    /** @arg with the top-up the forced cover fills in full on the cash-poor account */
    const ShortCaseResult topped = run_case(true);
    const TradeRecord* open_tr = findFirstTrade(topped.trades, BUSINESS_SELL_SHORT);
    const TradeRecord* cover_tr = findFirstTrade(topped.trades, BUSINESS_BUY_SHORT);
    REQUIRE_NE(open_tr, nullptr);
    REQUIRE_NE(cover_tr, nullptr);
    CHECK_EQ(cover_tr->datetime, SHORT_GAP_COVER);
    CHECK_EQ(cover_tr->number, open_tr->number);
    CHECK_EQ(topped.short_left, 0.0);

    /** @arg the scenario was really cash-poor: the buyback exceeds the cash after the short */
    const price_t cover_money = cover_tr->realPrice * cover_tr->number * stk.unit();
    CHECK_GT(cover_money, open_tr->cash);

    /** @arg the deposit happens at the forced cover bar and covers exactly the gap */
    const TradeRecord* checkin_tr = nullptr;
    for (const auto& tr : topped.trades) {
        if (tr.business == BUSINESS_CHECKIN) {
            checkin_tr = &tr;
        }
    }
    REQUIRE_NE(checkin_tr, nullptr);
    CHECK_EQ(checkin_tr->datetime, SHORT_GAP_COVER);
    CHECK_LT(std::fabs(checkin_tr->realPrice - (cover_money - open_tr->cash)), 0.01);

    /** @arg a forced long sell needs no cash: no deposit happens at the force bar (the entry top-up
     * is the only one) */
    SYSPtr long_sys = SYS_Simple();
    long_sys->setParam<bool>("buy_delay", false);
    long_sys->setParam<bool>("sell_delay", false);
    long_sys->setTM(crtTM(Datetime(199001010000LL), 9000, TC_Zero(), "TEST_TM"));
    long_sys->setSG(make_shared<ScriptedBuySG>(SHORT_GAP_OPEN));
    MMPtr long_mm = MM_FixedCount(SHORT_GAP_NUM);
    long_mm->setParam<bool>("auto_checkin", true);
    long_sys->setMM(long_mm);
    long_sys->run(k);
    REQUIRE_UNARY(long_sys->getTM()->getPosition(SHORT_GAP_COVER, stk).number > 0.0);

    long_sys->sellForceOnClose(SHORT_GAP_COVER, MAX_DOUBLE, PART_PORTFOLIO);
    const TradeRecordList& long_trades = long_sys->getTM()->getTradeList();
    CHECK_UNARY(findFirstTrade(long_trades, BUSINESS_SELL) != nullptr);
    CHECK_EQ(long_sys->getTM()->getPosition(SHORT_GAP_COVER, stk).number, 0.0);
    bool force_deposit = false;
    for (const auto& tr : long_trades) {
        force_deposit =
          force_deposit || (tr.business == BUSINESS_CHECKIN && tr.datetime == SHORT_GAP_COVER);
    }
    CHECK_UNARY(!force_deposit);
}

/** @} */
