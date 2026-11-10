/*
 * test_SYS_Simple.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include "test_sys.h"
#include <map>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/trade_manage/crtTM.h>
#include <hikyuu/trade_manage/margin/crt/MG_Fixed.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/signal/crt/SG_Manual.h>
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

/** @par Test point (the basic operations) */
TEST_CASE("test_SYS_Simple_for_base") {
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
    SYSPtr sys;

    /** @arg Run without a given account */
    sys = SYS_Simple();
    CHECK_THROWS_AS(sys->readyForRun(), std::exception);

    /** @arg The account is given but the other strategy components are not */
    sys = SYS_Simple();
    sys->setTM(tm->clone());
    CHECK_THROWS_AS(sys->readyForRun(), std::exception);

    /** @arg TM and SG are given but the other components are not */
    sys = SYS_Simple();
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    CHECK_THROWS_AS(sys->readyForRun(), std::exception);

    /** @arg TM, SG and MM are given but the others are not, a non-delayed operation */
    sys = SYS_Simple();
    sys->setParam("buy_delay", false);
    sys->setParam("sell_delay", false);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
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
    CHECK_LT(std::fabs(tr_list[1].stoploss - 0), 0.00001);
    current_cash = init_cash - 2645;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(199912220000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 25.15), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 25.15), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 0), 0.00001);
    current_cash += 2515;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    CHECK_EQ(tr_list[3].stock, stk);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001050000LL));
    CHECK_EQ(tr_list[3].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[3].planPrice - 25.28), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].realPrice - 25.28), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    CHECK_EQ(tr_list[3].number, 100);
    CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].stoploss - 0), 0.00001);
    current_cash -= 2528;
    CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[3].from, PART_SIGNAL);

    /** @arg TM, SG and MM are given but the others are not, a delayed operation */
    sys = SYS_Simple();
    sys->setParam("buy_delay", true);
    sys->setParam("sell_delay", true);
    sys->setTM(tm->clone());
    sys->setSG(sg->clone());
    sys->setMM(mm->clone());
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
    CHECK_LT(std::fabs(tr_list[1].stoploss - 0), 0.00001);
    current_cash = init_cash - 2650;
    CHECK_LT(std::fabs(tr_list[1].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[1].from, PART_SIGNAL);

    CHECK_EQ(tr_list[2].stock, stk);
    CHECK_EQ(tr_list[2].datetime, Datetime(199912230000LL));
    CHECK_EQ(tr_list[2].business, BUSINESS_SELL);
    CHECK_LT(std::fabs(tr_list[2].planPrice - 25.20), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].realPrice - 25.20), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[2].goalPrice));
    CHECK_EQ(tr_list[2].number, 100);
    CHECK_LT(std::fabs(tr_list[2].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[2].stoploss - 0), 0.00001);
    current_cash += 2520;
    CHECK_LT(std::fabs(tr_list[2].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[2].from, PART_SIGNAL);

    CHECK_EQ(tr_list[3].stock, stk);
    CHECK_EQ(tr_list[3].datetime, Datetime(200001060000LL));
    CHECK_EQ(tr_list[3].business, BUSINESS_BUY);
    CHECK_LT(std::fabs(tr_list[3].planPrice - 25.18), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].realPrice - 25.18), 0.00001);
    CHECK_UNARY(std::isnan(tr_list[3].goalPrice));
    CHECK_EQ(tr_list[3].number, 100);
    CHECK_LT(std::fabs(tr_list[3].cost.total - 0), 0.00001);
    CHECK_LT(std::fabs(tr_list[3].stoploss - 0), 0.00001);
    current_cash -= 2518;
    CHECK_LT(std::fabs(tr_list[3].cash - current_cash), 0.00001);
    CHECK_EQ(tr_list[3].from, PART_SIGNAL);

    /*for (auto iter = tr_list.begin(); iter != tr_list.end(); ++iter) {
        std::cout ,, *iter ,, std::endl;
    }*/
}

/** @par Test point (the auto_checkin deposits only at the actual trade of a delayed buy) */
TEST_CASE("test_SYS_Simple_for_base_auto_checkin") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    REQUIRE_UNARY(!stk.isNull());
    Datetime start_date(199911100000LL);
    Datetime end_date(200002250000LL);
    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    // A quantity far beyond the initial cash forces a top-up on every buy; a deposit on a date
    // without a trade is the regression signature of the old query-side deposit
    SYSPtr sys = SYS_Simple();
    sys->setParam("buy_delay", true);
    sys->setParam("sell_delay", true);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    MMPtr mm = MM_FixedCount(100000);
    mm->setParam<bool>("auto_checkin", true);
    sys->setMM(mm);

    sys->run(stk, query);

    TradeRecordList tr_list = sys->getTM()->getTradeList();
    std::map<Datetime, size_t> buy_counts, checkin_counts;
    for (const auto& tr : tr_list) {
        if (BUSINESS_BUY == tr.business) {
            buy_counts[tr.datetime]++;
        } else if (BUSINESS_CHECKIN == tr.business) {
            checkin_counts[tr.datetime]++;
        }
    }

    /** @arg the scenario really exercised the top-up path */
    CHECK_UNARY(buy_counts.size() >= 1);
    CHECK_UNARY(!checkin_counts.empty());

    /** @arg every deposit coincides with an actual buy */
    for (const auto& kv : checkin_counts) {
        CHECK_UNARY(buy_counts.count(kv.first) > 0);
    }

    /** @arg at most one deposit per buy */
    for (const auto& kv : checkin_counts) {
        CHECK_LE(kv.second, buy_counts[kv.first]);
    }
}

/** @par Test point (auto_checkin deposits only the margin of a margin-accounted buy) */
TEST_CASE("test_SYS_Simple_for_base_auto_checkin_margin") {
    StockManager& sm = StockManager::instance();
    Stock stk = sm["sh600000"];
    REQUIRE_UNARY(!stk.isNull());
    Datetime start_date(199911100000LL);
    Datetime end_date(200002250000LL);
    KQuery query = KQueryByDate(start_date, end_date, KQuery::DAY);

    // A cash-poor margin account: every buy needs a top-up, but only for its margin (the rest is
    // financed), so the deposit must stay within the margin of the buy it serves
    TMPtr tm = crtTM(Datetime(199001010000LL), 10000, TC_Zero(), "TEST_TM", MG_Fixed(0.5));
    tm->setParam<bool>("margin_accounting", true);
    // An A-share margin account needs the financing switch, otherwise the whole notional is
    // charged from the cash (isMarginAccounted)
    tm->setParam<bool>("support_borrow_cash", true);

    /** @arg the account really books this security by margin */
    CHECK_UNARY(tm->isMarginAccounted(stk));

    SYSPtr sys = SYS_Simple();
    // Non-delayed: at most one order per bar, so every deposit pairs with its own buy
    sys->setParam("buy_delay", false);
    sys->setParam("sell_delay", false);
    // The system leaves the financing switches off: the margin account it is given must keep them,
    // a system that silently disabled them would drop the whole book back to the cash accounting
    sys->setTM(tm);
    sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    MMPtr mm = MM_FixedCount(1000);
    mm->setParam<bool>("auto_checkin", true);
    sys->setMM(mm);

    sys->run(stk, query);

    std::map<Datetime, price_t> max_buy_margin, checkin_cash;
    std::map<Datetime, size_t> checkin_count;
    TradeRecordList tr_list = tm->getTradeList();
    for (const auto& tr : tr_list) {
        if (BUSINESS_BUY == tr.business) {
            price_t margin =
              tm->getOpenMargin(tr.datetime, tr.stock, tr.realPrice, tr.number, MarginSide::LONG);
            auto it = max_buy_margin.find(tr.datetime);
            if (it == max_buy_margin.end()) {
                max_buy_margin[tr.datetime] = margin;
            } else {
                it->second = std::max(it->second, margin);
            }
        } else if (BUSINESS_CHECKIN == tr.business) {
            checkin_cash[tr.datetime] += tr.realPrice;
            checkin_count[tr.datetime]++;
        }
    }

    /** @arg the scenario really exercised the top-up path of a margin-accounted buy */
    CHECK_UNARY(!max_buy_margin.empty());
    CHECK_UNARY(!checkin_cash.empty());

    /** @arg every single deposit stays within the margin of one buy of its date: the rest of the
     *  notional is financed, so depositing it would inflate the principal */
    size_t matched = 0;
    for (const auto& kv : checkin_cash) {
        auto it = max_buy_margin.find(kv.first);
        // A rejected buy leaves its deposit idle in the cash, lowering the next gap instead
        if (it == max_buy_margin.end()) {
            continue;
        }
        ++matched;
        CHECK_LE(kv.second, it->second * checkin_count[kv.first] + 1e-6);
    }
    CHECK_UNARY(matched > 0);
}

namespace {

/**
 * Overrides the variety-level contract multiplier of a loaded security (the runtime override of
 * Stock::tickValue; the fixture keeps the neutral type-level unit of 1, so a missing multiplication
 * would stay invisible). It changes the data shared by the whole process, so the cases may not run
 * in parallel
 */
class ScopedMultiplier {
public:
    ScopedMultiplier(Stock stock, double multiplier)
    : m_stock(stock), m_tickValue(stock.tickValue()) {
        m_stock.tickValue(m_tickValue * multiplier);
    }
    ~ScopedMultiplier() {
        m_stock.tickValue(m_tickValue);
    }

private:
    Stock m_stock;
    price_t m_tickValue;
};

}  // namespace

/** @par Test point (a margin-accounted futures buy tops up and locks the multiplied margin) */
TEST_CASE("test_SYS_Simple_for_base_margin_futures_multiplier") {
    StockManager& sm = StockManager::instance();
    Stock rb = sm["SHFRB0000"];
    REQUIRE_UNARY(!rb.isNull());
    REQUIRE_EQ(rb.unit(), 1.0);

    static const double MULTIPLIER = 10.0;  // rebar: 10 tons per lot
    ScopedMultiplier scoped(rb, MULTIPLIER);
    REQUIRE_EQ(rb.unit(), MULTIPLIER);

    KQuery query = KQueryByIndex(-6, Null<int64_t>(), KQuery::DAY);
    KData k = rb.getKData(query);
    REQUIRE_UNARY(k.size() >= 3);
    // One bar after the signal, so the order is executed inside the run instead of at its edge
    Datetime dt = k[k.size() - 2].datetime;
    price_t close = k[k.size() - 2].closePrice;
    REQUIRE_UNARY(close > 0.0);

    double number = 5.0;
    price_t margin = roundEx(0.5 * close * number * MULTIPLIER, 2);
    REQUIRE_UNARY(margin > 0.0);

    // The account keeps half of the margin: the top-up has to cover the rest of the margin, while
    // the full notional (MULTIPLIER times bigger) must never be deposited
    TMPtr tm = crtTM(Datetime(199001010000LL), margin / 2, TC_Zero(), "TEST_TM", MG_Fixed(0.5));
    tm->setParam<bool>("margin_accounting", true);

    SGPtr sg = SG_Manual();
    sg->_addBuySignal(dt);
    MMPtr mm = MM_FixedCount(number);
    mm->setParam<bool>("auto_checkin", true);

    SYSPtr sys = SYS_Simple();
    sys->setParam("buy_delay", false);
    sys->setParam("sell_delay", false);
    sys->setTM(tm);
    sys->setSG(sg);
    sys->setMM(mm);
    sys->run(rb, query);

    /** @arg the order really went through at the multiplied size */
    REQUIRE_UNARY(tm->have(rb));
    PositionRecord pos = tm->getPosition(dt, rb);
    CHECK_EQ(pos.number, doctest::Approx(number));

    /** @arg the lock is the margin of the multiplied notional on the opening day */
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).occupied_margin, doctest::Approx(margin));

    /** @arg the daily settlement of the following bars then relocks the holding at their close */
    price_t last_close = k[k.size() - 1].closePrice;
    CHECK_EQ(pos.margin, doctest::Approx(roundEx(0.5 * last_close * number * MULTIPLIER, 2)));

    /** @arg the deposit covers the missing half of the margin only */
    price_t checkin = 0.0;
    for (const auto& tr : tm->getTradeList()) {
        if (tr.business == BUSINESS_CHECKIN) {
            checkin += tr.realPrice;
        }
    }
    CHECK_GT(checkin, 0.0);
    CHECK_EQ(checkin, doctest::Approx(margin - margin / 2));

    /** @arg the notional never entered the cash, so the equity is exactly the capital paid in: an
     *  accounting identity, the multiplier itself is pinned by the absolute amounts above */
    CHECK_EQ(tm->getFunds(dt, KQuery::DAY).net_assets(), doctest::Approx(margin / 2 + checkin));

    /** @arg off the accounting the same buy tops up the whole multiplied notional instead of only
     *  its margin */
    price_t notional = roundEx(close * number * MULTIPLIER, 2);
    TMPtr tm2 = crtTM(Datetime(199001010000LL), margin / 2, TC_Zero(), "TEST_TM2", MG_Fixed(0.5));
    SGPtr sg2 = SG_Manual();
    sg2->_addBuySignal(dt);
    MMPtr mm2 = MM_FixedCount(number);
    mm2->setParam<bool>("auto_checkin", true);
    SYSPtr sys2 = SYS_Simple();
    sys2->setParam("buy_delay", false);
    sys2->setParam("sell_delay", false);
    sys2->setTM(tm2);
    sys2->setSG(sg2);
    sys2->setMM(mm2);
    sys2->run(rb, query);

    price_t checkin2 = 0.0;
    for (const auto& tr : tm2->getTradeList()) {
        if (tr.business == BUSINESS_CHECKIN) {
            checkin2 += tr.realPrice;
        }
    }
    REQUIRE_UNARY(tm2->have(rb));
    CHECK_EQ(checkin2, doctest::Approx(notional - margin / 2));
    CHECK_EQ(tm2->getPosition(dt, rb).margin, doctest::Approx(0.0));
}

/** @} */
