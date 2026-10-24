/*
 * test_SYS_Simple.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include "test_sys.h"
#include <cmath>
#include <map>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>
#include <hikyuu/trade_sys/moneymanager/MoneyManagerBase.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_FixedPercent.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_Indicator.h>
#include <hikyuu/indicator/crt/KDATA.h>

using namespace hku;

/**
 * @defgroup test_SYS_Simple test_SYS_Simple
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test point (the take-profit strategy) */
TEST_CASE("test_SYS_Simple_for_tp") {
    TradeRecordList tr_list;
    price_t current_cash;

    StockManager& sm = StockManager::instance();

    // The initial parameters
    Datetime init_date(199001010000LL);   // Account initial date
    price_t init_cash = 100000;           // Account initial amount
    TradeCostPtr costfunc = TC_Zero();    // The zero cost function
    Stock stk = sm["sh600000"];           // The selected security
    Datetime start_date(199911100000LL);  // Test start date
    Datetime end_date(200002250000LL);    // Test end date

    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    // Build the system parts
    TMPtr tm = crtTM(init_date, init_cash, costfunc, "TEST_TM");
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);
    STPtr st = ST_FixedPercent(0.01);
    TPPtr tp = ST_Indicator(MA(CLOSE(), 5));
    SYSPtr sys;

    /** @arg TM, SG, MM, ST and TP are given but the others are not, a non-delayed operation */
    sys = SYS_Simple();
    sys->setParam("buy_delay", false);
    sys->setParam("sell_delay", false);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setST(st->clone());
    sys->setTP(tp->clone());
    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_NE(tr_list.size(), 1);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(199912150000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 26.45), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 26.45), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 26.19), 0.00001);
    current_cash = init_cash - 2645;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    // CHECK_EQ(tr_list[2].datetime, Datetime(199912160000LL));
    // CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    // CHECK_LT(std::fabs(tr_list[2].planPrice - 26.00), 0.00001);
    // CHECK_LT(std::fabs(tr_list[2].realPrice - 26.00), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    // CHECK_EQ(tr_list[2].number, 100);
    // CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[2].stoploss - 25.74), 0.00001);
    // current_cash += 2600;
    // CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[2].from, PART_STOPLOSS);

    // CHECK_EQ(tr_list[3].stock, stk);
    // CHECK_EQ(tr_list[3].datetime, Datetime(200001050000LL));
    // CHECK_EQ(tr_list[3].business, BUSINESS_BUY);
    // CHECK_LT(std::fabs(tr_list[3].planPrice - 25.28), 0.00001);
    // CHECK_LT(std::fabs(tr_list[3].realPrice - 25.28), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    // CHECK_EQ(tr_list[3].number, 100);
    // CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[3].stoploss - 25.03), 0.00001);
    // current_cash -= 2528;
    // CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[3].from, PART_SIGNAL);

    // CHECK_EQ(tr_list[4].stock, stk);
    // CHECK_EQ(tr_list[4].datetime, Datetime(200001110000LL));
    // CHECK_EQ(tr_list[4].business, BUSINESS_SELL);
    // CHECK_LT(std::fabs(tr_list[4].planPrice - 26.20), 0.00001);
    // CHECK_LT(std::fabs(tr_list[4].realPrice - 26.20), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[4].goalPrice));
    // CHECK_EQ(tr_list[4].number, 100);
    // CHECK_LT(std::fabs(tr_list[4].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[4].stoploss - 25.94), 0.00001);
    // current_cash += 2620;
    // CHECK_LT(std::fabs(tr_list[4].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[4].from, PART_TAKEPROFIT);

    /** @arg TM, SG, MM, ST and TP are given but the others are not, a delayed operation */
    sys = SYS_Simple();
    sys->setParam("buy_delay", true);
    sys->setParam("sell_delay", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
    sys->setST(st->clone());
    sys->setTP(tp->clone());
    sys->run(stk, query);
    CHECK_NE(sys->getTM()->currentCash(), init_cash);
    tr_list = sys->getTM()->getTradeList();
    CHECK_NE(tr_list.size(), 1);
    CHECK_EQ(tr_list[0].business, BUSINESS_INIT);

    CHECK_EQ(tr_list[1].stock, stk);
    CHECK_EQ(tr_list[1].datetime, Datetime(199912160000LL));
    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[1].planPrice - 26.50), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].realPrice - 26.50), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[1].goalPrice));
    CHECK_EQ(tr_list[1].number, 100);
    CHECK_LT(std::fabs(tr_list[1].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[1].stoploss - 26.24), 0.00001);
    current_cash = init_cash - 2650;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    // CHECK_EQ(tr_list[2].datetime, Datetime(199912170000LL));
    // CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    // CHECK_LT(std::fabs(tr_list[2].planPrice - 26.00), 0.00001);
    // CHECK_LT(std::fabs(tr_list[2].realPrice - 26.00), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    // CHECK_EQ(tr_list[2].number, 100);
    // CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[2].stoploss - 25.74), 0.00001);
    // current_cash += 2600;
    // CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[2].from, PART_STOPLOSS);

    // CHECK_EQ(tr_list[3].stock, stk);
    // CHECK_EQ(tr_list[3].datetime, Datetime(200001060000LL));
    // CHECK_EQ(tr_list[3].business, BUSINESS_BUY);
    // CHECK_LT(std::fabs(tr_list[3].planPrice - 25.18), 0.00001);
    // CHECK_LT(std::fabs(tr_list[3].realPrice - 25.18), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    // CHECK_EQ(tr_list[3].number, 100);
    // CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[3].stoploss - 24.93), 0.00001);
    // current_cash -= 2518;
    // CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[3].from, PART_SIGNAL);

    // CHECK_EQ(tr_list[4].stock, stk);
    // CHECK_EQ(tr_list[4].datetime, Datetime(200001120000LL));
    // CHECK_EQ(tr_list[4].business, BUSINESS_SELL);
    // CHECK_LT(std::fabs(tr_list[4].planPrice - 26.00), 0.00001);
    // CHECK_LT(std::fabs(tr_list[4].realPrice - 26.00), 0.00001);
    // CHECK_UNARY(std::isnan(tr_list[4].goalPrice));
    // CHECK_EQ(tr_list[4].number, 100);
    // CHECK_LT(std::fabs(tr_list[4].cost.total - 0), 0.00001);
    // CHECK_LT(std::fabs(tr_list[4].stoploss - 25.74), 0.00001);
    // current_cash += 2600;
    // CHECK_LT(std::fabs(tr_list[4].cash - current_cash), 0.00001);
    // CHECK_EQ(tr_list[4].from, PART_TAKEPROFIT);
}

//-----------------------------------------------------------------------------
// The take-profit ratchet on a partial close: a partial close keeps the high-water
// mark; only a flat book resets the ratchet to 0
//-----------------------------------------------------------------------------

namespace {

// A money manager buying 200 shares once and selling 100 per exit, so a TP close is partial
class TpPartialMM : public MoneyManagerBase {
public:
    TpPartialMM() : MoneyManagerBase("TP_PARTIAL_MM") {}

    virtual MoneyManagerPtr _clone() override {
        return make_shared<TpPartialMM>();
    }

    virtual double _getBuyNumber(const Datetime&, const Stock&, price_t, price_t,
                                 SystemPart) override {
        return 200.0;
    }

    virtual double _getSellNumber(const Datetime&, const Stock&, price_t, price_t,
                                  SystemPart) override {
        return 100.0;
    }
};

// Buys once at the bar given at construction; the exits are driven by the TP alone
class TpBuyOnceSG : public SignalBase {
public:
    explicit TpBuyOnceSG(const Datetime& date) : SignalBase("TP_BUY_ONCE_SG"), m_date(date) {}

    virtual SignalPtr _clone() override {
        return make_shared<TpBuyOnceSG>(m_date);
    }

    virtual void _calculate(const KData& kdata) override {
        if (kdata.getPos(m_date) != Null<size_t>()) {
            _addBuySignal(m_date);
        }
    }

private:
    Datetime m_date;
};

// A take-profit carrying a per-date level; 0 means no level on that bar
class TpLevelsTP : public StoplossBase {
public:
    explicit TpLevelsTP(const std::map<Datetime, price_t>& levels)
    : StoplossBase("TP_LEVELS_TP"), m_levels(levels) {}

    virtual StoplossPtr _clone() override {
        return make_shared<TpLevelsTP>(m_levels);
    }

    virtual price_t _getPrice(const Datetime& datetime, price_t) override {
        auto iter = m_levels.find(datetime);
        return iter != m_levels.end() ? iter->second : 0.0;
    }

private:
    std::map<Datetime, price_t> m_levels;
};

// Exposes the long take-profit ratchet for state assertions
class TpRatchetProbeSYS : public System {
public:
    TpRatchetProbeSYS() : System("TpRatchetProbeSYS") {}

    price_t longRatchet() const {
        return m_lastTakeProfit;
    }
};

// The entry bar is the window minimum close, so a later rise over the entry (required by the
// take-profit profit gate) is guaranteed to exist
size_t tpEntryPos(const KData& k) {
    size_t e = 0;
    for (size_t i = 1; i < k.size(); ++i) {
        if (k[i].closePrice < k[e].closePrice) {
            e = i;
        }
    }
    return e;
}

std::shared_ptr<TpRatchetProbeSYS> makeTpProbeSys(const KData& k, size_t entryPos,
                                                  const StoplossPtr& tp, bool sellDelay) {
    auto sys = std::make_shared<TpRatchetProbeSYS>();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", sellDelay);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(make_shared<TpBuyOnceSG>(k[entryPos].datetime));
    sys->setMM(make_shared<TpPartialMM>());
    sys->setTP(tp);
    sys->setTO(k);
    sys->readyForRun();
    return sys;
}

}  // namespace

/** @par Check points (the take-profit ratchet on a partial close):
 *  the first partial close keeps the ratchet at its level instead of resetting it to the fill, so
 *  a later rebound below that level still closes the second tranche */
TEST_CASE("test_SYS_Simple_tp_partial_close_keeps_ratchet") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh600000"];
    KData k =
      stock.getKData(KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY));
    REQUIRE(k.size() > 10);
    const size_t e = tpEntryPos(k);

    // t1: the first tradable rise over the entry close; the level h1 = close[t1] * 1.06 fires
    // there and lifts the ratchet to h1 (above the entry fill). t2 (> t1): a mild rebound,
    // close[t1] * 1.02 < close[t2] <= close[t1] * 1.04; the level h2 = close[t1] * 1.001 sits
    // between the t1 fill and the ratchet h1, so with the kept ratchet the rebound closes the
    // second tranche, while a reset-to-fill ratchet (max(h2, fill) = h2) stays silent. Both
    // outcomes keep a margin of about 2% to the level that decides them
    size_t t1 = 0;
    size_t t2 = 0;
    for (size_t i = e + 1; i + 1 < k.size() && t2 == 0; ++i) {
        const KRecord& r1 = k[i];
        if (r1.closePrice <= k[e].closePrice || r1.highPrice == r1.lowPrice ||
            r1.transAmount == 0.0) {
            continue;
        }
        for (size_t j = i + 1; j < k.size(); ++j) {
            const KRecord& r2 = k[j];
            if (r2.highPrice == r2.lowPrice || r2.transAmount == 0.0) {
                continue;
            }
            if (r2.closePrice > r1.closePrice * 1.02 && r2.closePrice <= r1.closePrice * 1.04) {
                t1 = i;
                t2 = j;
                break;
            }
        }
    }
    REQUIRE_UNARY(t2 != 0);

    const price_t h1 = k[t1].closePrice * 1.06;
    std::map<Datetime, price_t> levels;
    levels[k[t1].datetime] = h1;
    levels[k[t2].datetime] = k[t1].closePrice * 1.001;

    auto sys = makeTpProbeSys(k, e, make_shared<TpLevelsTP>(levels), false);

    for (size_t i = 0; i <= t1; ++i) {
        sys->runMoment(k[i].datetime);
    }

    TradeRecordList tr_list = sys->getTM()->getTradeList();
    REQUIRE_EQ(tr_list.size(), 3);  // INIT + BUY + the first TP sell

    CHECK_EQ(tr_list[1].business, BUSINESS_BUY);
    CHECK_EQ(tr_list[1].number, 200.0);

    /** @arg the first partial close fires on the h1 bar */
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_EQ(tr_list[2].datetime, k[t1].datetime);
    CHECK_EQ(tr_list[2].number, 100.0);
    CHECK_EQ(tr_list[2].from, PART_TAKEPROFIT);

    /** @arg the ratchet after the partial close is the level that fired, not the fill price */
    CHECK_LT(std::fabs(sys->longRatchet() - h1), h1 * 0.005);

    for (size_t i = t1 + 1; i <= t2; ++i) {
        sys->runMoment(k[i].datetime);
    }

    tr_list = sys->getTM()->getTradeList();
    REQUIRE_EQ(tr_list.size(), 4);
    /** @arg the second partial close still fires on the rebound below the kept ratchet */
    CHECK_EQ(tr_list[3].business, BUSINESS_SELL);
    CHECK_EQ(tr_list[3].datetime, k[t2].datetime);
    CHECK_EQ(tr_list[3].number, 100.0);
    CHECK_EQ(tr_list[3].from, PART_TAKEPROFIT);
    CHECK_UNARY_FALSE(sys->getTM()->have(stock));

    /** @arg a flat book resets the ratchet to 0 */
    CHECK_EQ(sys->longRatchet(), 0.0);
}

/** @par Check points (a delayed partial close keeps the ratchet):
 *  the sell request submitted on the take-profit bar is filled at the next open, which must not
 *  reset the ratchet to the open price */
TEST_CASE("test_SYS_Simple_tp_partial_close_keeps_ratchet_when_delayed") {
    StockManager& sm = StockManager::instance();
    Stock stock = sm["sh600000"];
    KData k =
      stock.getKData(KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL), KQuery::DAY));
    REQUIRE(k.size() > 10);
    const size_t e = tpEntryPos(k);

    // The first tradable rise over the entry close, so the take-profit profit gate is met
    size_t t1 = 0;
    for (size_t i = e + 1; i + 1 < k.size(); ++i) {
        const KRecord& r = k[i];
        if (r.closePrice > k[e].closePrice && r.highPrice != r.lowPrice && r.transAmount != 0.0) {
            t1 = i;
            break;
        }
    }
    REQUIRE_UNARY(t1 != 0);

    const price_t h1 = k[t1].closePrice * 1.05;
    std::map<Datetime, price_t> levels;
    levels[k[t1].datetime] = h1;

    auto sys = makeTpProbeSys(k, e, make_shared<TpLevelsTP>(levels), true);

    for (size_t i = 0; i <= t1; ++i) {
        sys->runMoment(k[i].datetime);
    }

    TradeRecordList tr_list = sys->getTM()->getTradeList();
    REQUIRE_EQ(tr_list.size(), 2);  // INIT + BUY: the sell is only a pending request
    CHECK_LT(std::fabs(sys->longRatchet() - h1), h1 * 0.005);

    sys->runMoment(k[t1 + 1].datetime);

    tr_list = sys->getTM()->getTradeList();
    REQUIRE_EQ(tr_list.size(), 3);
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_EQ(tr_list[2].datetime, k[t1 + 1].datetime);
    CHECK_EQ(tr_list[2].number, 100.0);
    CHECK_EQ(tr_list[2].from, PART_TAKEPROFIT);

    /** @arg the delayed partial fill keeps the ratchet instead of resetting it to the open price */
    CHECK_UNARY(sys->getTM()->have(stock));
    CHECK_LT(std::fabs(sys->longRatchet() - h1), h1 * 0.005);
}

/** @} */
