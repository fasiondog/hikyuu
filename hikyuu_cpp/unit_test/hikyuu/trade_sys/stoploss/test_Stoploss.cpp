/*
 * test_Stoploss.cpp
 *
 *  Created on: 2013-3-13
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/indicator/build_in.h>
#include <hikyuu/trade_manage/crt/TC_Zero.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/stoploss/StoplossBase.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_FixedPercent.h>
#include <hikyuu/trade_sys/stoploss/crt/ST_Indicator.h>
#include <hikyuu/trade_sys/stoploss/imp/FixedPercentStoploss.h>
#include <hikyuu/trade_sys/stoploss/imp/IndicatorStoploss.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>

using namespace hku;

class StoplossTest : public StoplossBase {
public:
    StoplossTest() : StoplossBase("StoplossTest"), m_x(0) {}
    virtual ~StoplossTest() {}

    virtual price_t _getPrice(const Datetime &datetime, price_t price) {
        return m_x < 10 ? 0.0 : 1.0;
    }

    virtual void _reset() {
        m_x = 0;
    }

    virtual StoplossPtr _clone() {
        StoplossTest *p = new StoplossTest;
        p->m_x = m_x;
        return StoplossPtr(p);
    }

    virtual void _calculate() {}

    int getX() const {
        return m_x;
    }
    void setX(int x) {
        m_x = x;
    }

private:
    int m_x;
};

class StoplossAccum : public StoplossBase {
public:
    StoplossAccum() : StoplossBase("StoplossAccum") {}
    virtual ~StoplossAccum() {}

    virtual price_t _getPrice(const Datetime &, price_t) override {
        return 0.0;
    }
    virtual void _reset() override {
        m_count = 0;
    }
    virtual void _calculate() override {
        m_count += static_cast<int>(m_kdata.size());
    }
    virtual StoplossPtr _clone() override {
        auto p = make_shared<StoplossAccum>();
        p->m_count = m_count;
        return p;
    }

    int count() const {
        return m_count;
    }

private:
    int m_count{0};
};

// A subclass that, like Condition/Environment subclasses, copies its cached calculation result in
// _clone(); the base clone() copies m_kdata, so the clone is immediately usable (EV/CN convention).
class StoplossCloneWithCache : public StoplossBase {
public:
    StoplossCloneWithCache() : StoplossBase("StoplossCloneWithCache"), m_calc(0) {
        setParam<int>("offset", 0);
    }
    virtual ~StoplossCloneWithCache() {}
    virtual price_t _getPrice(const Datetime &, price_t) override {
        return static_cast<price_t>(m_calc);
    }
    virtual void _reset() override {
        m_calc = -1;
    }
    virtual void _calculate() override {
        m_calc = static_cast<int>(m_kdata.size()) + getParam<int>("offset");
    }
    virtual StoplossPtr _clone() override {
        auto p = make_shared<StoplossCloneWithCache>();
        p->m_calc = m_calc;
        return p;
    }

private:
    int m_calc{0};
};

// A subclass returning an arbitrary raw value, used to verify the base-class normalization
class StoplossInvalidReturn : public StoplossBase {
public:
    StoplossInvalidReturn() : StoplossBase("StoplossInvalidReturn") {}
    virtual ~StoplossInvalidReturn() {}

    virtual price_t _getPrice(const Datetime &, price_t) override {
        return m_value;
    }
    virtual price_t _getShortPrice(const Datetime &, price_t) override {
        return m_value;
    }
    virtual StoplossPtr _clone() override {
        auto p = make_shared<StoplossInvalidReturn>();
        p->m_value = m_value;
        return p;
    }
    virtual void _calculate() override {}

    void setValue(price_t value) {
        m_value = value;
    }

private:
    price_t m_value{0.0};
};

/**
 * @defgroup test_Stoploss test_Stoploss
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Stoploss") {
    /** @arg The basic operation */
    StoplossPtr p(new StoplossTest);
    CHECK_EQ(p->name(), "StoplossTest");
    CHECK_EQ(p->getPrice(Datetime(200101010000), 1.0), 0.0);
    StoplossTest *p_src = (StoplossTest *)p.get();
    CHECK_EQ(p_src->getX(), 0);

    p_src->setX(10);
    CHECK_EQ(p->getPrice(Datetime(200101010000), 1.0), 1.0);
    CHECK_EQ(p_src->getX(), 10);
    p->reset();
    CHECK_EQ(p_src->getX(), 0);

    /** @arg Test the clone operation */
    p_src->setX(10);
    StoplossPtr p_clone = p->clone();
    CHECK_EQ(p_clone->name(), "StoplossTest");
    p_src = (StoplossTest *)p_clone.get();
    CHECK_EQ(p_src->getX(), 10);
    CHECK_NE(p, p_clone);
}

/** @arg setTO clears the subclass cache (_reset) before _calculate, so switching KData does not
 * accumulate stale results */
TEST_CASE("test_Stoploss_setTO_reset_before_calculate") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    KData k2 = sm.getStock("sz000001").getKData(KQuery(0, 5));
    CHECK_UNARY(!k1.empty());
    CHECK_UNARY(!k2.empty());

    auto p = make_shared<StoplossAccum>();
    p->setTO(k1);
    CHECK_EQ(p->count(), static_cast<int>(k1.size()));

    p->setTO(k2);
    CHECK_EQ(p->count(), static_cast<int>(k2.size()));
}

/** @arg reset drops the calculated flag, so a later setTO recomputes instead of early-returning
 * into a "calculated but no data" state (SL10) */
TEST_CASE("test_Stoploss_reset_clears_calculated") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sz000001").getKData(KQuery(0, 10));
    CHECK_UNARY(!k.empty());

    auto st = ST_FixedPercent(0.01);
    st->setTO(k);
    CHECK_UNARY(std::isfinite(st->getPrice(k[0].datetime, k[0].closePrice)));

    // reset() must clear the calculated flag. Otherwise setTO(empty) could early-return (KData
    // equality treats two empty stocks as equal) and leave a stale "calculated but no data" state,
    // and a following real setTO would be skipped.
    st->reset();
    st->setTO(KData());
    st->setTO(k);
    for (size_t i = 0; i < k.size(); ++i) {
        CHECK_UNARY(std::isfinite(st->getPrice(k[i].datetime, k[i].closePrice)));
    }
}

/** @arg clone carries only the ST's own configuration: the source's trade manager and trading object
 * are not copied, since the System re-injects them (m_tm via readyForRun, m_kdata via setTO) (SL11) */
TEST_CASE("test_Stoploss_clone_does_not_copy_tm_kdata") {
    auto st = ST_FixedPercent(0.01);
    KData k = StockManager::instance().getStock("sz000001").getKData(KQuery(0, 10));
    auto tm = crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM");
    st->setTO(k);
    st->setTM(tm);
    CHECK_UNARY(st->getTM() != nullptr);
    CHECK_UNARY(!st->getTO().empty());
    auto st_clone = st->clone();
    CHECK_UNARY(st_clone->getTM() == nullptr);
    CHECK_UNARY(st_clone->getTO().empty());
}

/** @arg clone copies m_kdata and the subclass cache, so the clone is directly usable; setTO always
 * recalculates (lazy recalculation), with the same or a different KData alike */
TEST_CASE("test_Stoploss_clone_preserves_cache") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    KData k2 = sm.getStock("sz000001").getKData(KQuery(0, 5));
    CHECK_UNARY(!k1.empty());
    CHECK_UNARY(!k2.empty());

    auto p = make_shared<StoplossCloneWithCache>();
    p->setTO(k1);
    CHECK_EQ(p->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size()));

    StoplossPtr pc = p->clone();
    // The clone carries the cached result and is usable before setTO
    CHECK_EQ(pc->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size()));

    // setTO with the same KData recalculates instead of short-circuiting
    pc->setTO(k1);
    CHECK_EQ(pc->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size()));

    // A different KData triggers a fresh calculation
    pc->setTO(k2);
    CHECK_EQ(pc->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k2.size()));
}

/** @arg stop-loss is floored to the price tick and never rounds back up to the buy price (ST-001)
 */
TEST_CASE("test_Stoploss_fixedpercent_rounding") {
    auto st = ST_FixedPercent(0.002);
    // empty kdata -> precision 2; 2.50*(1-0.002)=2.495 -> floor -> 2.49 (roundEx would give 2.50)
    CHECK_EQ(st->getPrice(Datetime(200101010000), 2.50), 2.49);
    // 3.33*(1-0.001)=3.32667... -> floor -> 3.32
    CHECK_EQ(st->getPrice(Datetime(200101010000), 3.33), 3.32);

    auto st2 = ST_FixedPercent(0.03);
    CHECK_LT(st2->getPrice(Datetime(200101010000), 10.0), 10.0);
}

/** @arg short stop-loss is above the planned price and invalid input yields no stop-loss (ST-006)
 */
TEST_CASE("test_Stoploss_fixedpercent_short_and_validation") {
    auto st = ST_FixedPercent(0.03);
    // empty kdata -> precision 2; 10*(1+0.03)=10.3 -> roundUp -> 10.3 (above 10)
    CHECK_EQ(st->getShortPrice(Datetime(200101010000), 10.0), 10.3);
    CHECK_EQ(st->getPrice(Datetime(200101010000), 0.0), 0.0);
    CHECK_EQ(st->getShortPrice(Datetime(200101010000), 0.0), 0.0);
}

/** @arg non-finite indicator values are dropped, so every bar is invalid (Null) (ST-005) */
TEST_CASE("test_Stoploss_indicator_nan_filtered") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    CHECK_UNARY(!k1.empty());

    // CLOSE()/(CLOSE()-CLOSE()) divides by zero -> Inf/NaN at every bar
    Indicator nan_ind = CLOSE() / (CLOSE() - CLOSE());
    auto st = ST_Indicator(nan_ind);
    st->setTO(k1);
    for (size_t i = 0; i < k1.size(); ++i) {
        CHECK_UNARY(std::isnan(st->getPrice(k1[i].datetime, 0.0)));
    }
}

/** @arg bars before the indicator produces a value (warm-up) are invalid, later bars are valid */
TEST_CASE("test_Stoploss_indicator_warmup_is_invalid") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 20));
    CHECK_UNARY(!k1.empty());

    auto st = ST_Indicator(MA(CLOSE(), 5));
    st->setTO(k1);

    size_t first_valid = Null<size_t>();
    for (size_t i = 0; i < k1.size(); ++i) {
        if (std::isfinite(st->getPrice(k1[i].datetime, 0.0))) {
            first_valid = i;
            break;
        }
    }

    /** @arg the indicator needs several bars, so its warm-up bars must be invalid */
    CHECK_UNARY(first_valid != Null<size_t>());
    CHECK_UNARY(first_valid > 0);
    for (size_t i = 0; i < first_valid; ++i) {
        CHECK_UNARY(std::isnan(st->getPrice(k1[i].datetime, 0.0)));
    }

    /** @arg once the indicator has values, every bar keeps a valid level */
    for (size_t i = first_valid; i < k1.size(); ++i) {
        CHECK_UNARY(std::isfinite(st->getPrice(k1[i].datetime, 0.0)));
    }
}

/** @arg invalid hook results are marked as Null (invalid) so that the system skips the trade,
 * while 0 keeps meaning "no stop-loss" */
TEST_CASE("test_Stoploss_invalid_hook_result_is_null") {
    auto st = make_shared<StoplossInvalidReturn>();
    Datetime datetime(200101010000);

    /** @arg negative result is invalid */
    st->setValue(-1.0);
    CHECK_UNARY(std::isnan(st->getPrice(datetime, 10.0)));
    CHECK_UNARY(std::isnan(st->getShortPrice(datetime, 10.0)));

    /** @arg NaN (Null) result stays invalid */
    st->setValue(Null<price_t>());
    CHECK_UNARY(std::isnan(st->getPrice(datetime, 10.0)));
    CHECK_UNARY(std::isnan(st->getShortPrice(datetime, 10.0)));

    /** @arg infinity result is invalid */
    st->setValue(std::numeric_limits<price_t>::infinity());
    CHECK_UNARY(std::isnan(st->getPrice(datetime, 10.0)));
    CHECK_UNARY(std::isnan(st->getShortPrice(datetime, 10.0)));

    /** @arg zero still means no stop-loss */
    st->setValue(0.0);
    CHECK_EQ(st->getPrice(datetime, 10.0), 0.0);
    CHECK_EQ(st->getShortPrice(datetime, 10.0), 0.0);

    /** @arg a valid positive result is passed through unchanged */
    st->setValue(9.5);
    CHECK_EQ(st->getPrice(datetime, 10.0), 9.5);
    CHECK_EQ(st->getShortPrice(datetime, 10.0), 9.5);
}

/** @arg clone carries no result cache; after its own setTO it computes an independent result that does
 * not leak from the source's recomputation (ST-007). A standalone clone must be set before use,
 * consistent with the "clone copies only configuration" contract (SL11) */
TEST_CASE("test_Stoploss_indicator_clone_independent") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    KData k2 = sm.getStock("sz000001").getKData(KQuery(5, 15));
    CHECK_UNARY(!k1.empty());
    CHECK_UNARY(!k2.empty());

    auto st = ST_Indicator(CLOSE());
    st->setTO(k1);
    StoplossPtr st_clone = st->clone();
    // The clone carries no cached result; it must be set (System::readyForRun / setTO) before use.
    st->setTO(k2);
    st_clone->setTO(k1);
    // The clone computes its own result, independent of the source's recomputation.
    for (size_t i = 0; i < k1.size(); ++i) {
        CHECK_EQ(st_clone->getPrice(k1[i].datetime, 0.0), doctest::Approx(k1[i].closePrice));
    }
    for (size_t i = 0; i < k2.size(); ++i) {
        CHECK_EQ(st->getPrice(k2[i].datetime, 0.0), doctest::Approx(k2[i].closePrice));
    }
}

/** @arg paramChanged invalidates the cache, so setTO with the same KData recomputes (ST-009) */
TEST_CASE("test_Stoploss_paramChanged_invalidates_cache") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    CHECK_UNARY(!k1.empty());

    auto p = make_shared<StoplossCloneWithCache>();
    p->setTO(k1);
    CHECK_EQ(p->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size()));

    p->setParam<int>("offset", 7);
    // Same KData, but the parameter changed -> cache must be invalidated and recomputed
    p->setTO(k1);
    CHECK_EQ(p->getPrice(Datetime(200101010000), 0.0), static_cast<price_t>(k1.size() + 7));
}

namespace {

// Emits one buy signal at a fixed date
class StoplossBuySG : public SignalBase {
public:
    explicit StoplossBuySG(const Datetime &datetime)
    : SignalBase("Stoploss_BuySG"), m_datetime(datetime) {}

    virtual SignalPtr _clone() override {
        return make_shared<StoplossBuySG>(m_datetime);
    }

    virtual void _calculate(const KData &kdata) override {
        if (kdata.getPos(m_datetime) != Null<size_t>()) {
            _addBuySignal(m_datetime);
        }
    }

private:
    Datetime m_datetime;
};

SYSPtr makeStoplossSys(const KData &kdata, const StoplossPtr &st, const Datetime &buy_date) {
    SYSPtr sys = SYS_Simple();
    sys->setParam<bool>("buy_delay", false);
    sys->setParam<bool>("sell_delay", false);
    sys->setTM(crtTM(Datetime(199001010000LL), 100000, TC_Zero(), "TEST_TM"));
    sys->setSG(make_shared<StoplossBuySG>(buy_date));
    sys->setMM(MM_FixedCount(100));
    sys->setST(st);
    sys->run(kdata);
    return sys;
}

size_t countBuy(const SYSPtr &sys) {
    size_t count = 0;
    for (const auto &tr : sys->getTradeRecordList()) {
        if (tr.business == BUSINESS_BUY) {
            ++count;
        }
    }
    return count;
}

price_t buyStoploss(const SYSPtr &sys) {
    for (const auto &tr : sys->getTradeRecordList()) {
        if (tr.business == BUSINESS_BUY) {
            return tr.stoploss;
        }
    }
    return Null<price_t>();
}

}  // namespace

/** @arg an empty indicator does not crash and yields no stop-loss (SL07) */
TEST_CASE("test_Stoploss_indicator_empty_input_guard") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sz000001").getKData(KQuery(0, 10));
    CHECK_UNARY(!k1.empty());

    // ST_Indicator(Indicator()) has an empty underlying indicator. setTO must not throw (it used to
    // throw before the input guard), and every bar must stay invalid (Null) so the trade opens
    // without stop-loss.
    auto st = ST_Indicator(Indicator());
    CHECK_NOTHROW(st->setTO(k1));
    for (size_t i = 0; i < k1.size(); ++i) {
        CHECK_UNARY(std::isnan(st->getPrice(k1[i].datetime, 0.0)));
    }

    // The same must hold when the empty-indicator ST drives a System: the buy opens without
    // stop-loss instead of crashing or throwing at setTO.
    Datetime buy_date = k1[0].datetime;
    auto sys = makeStoplossSys(k1, ST_Indicator(Indicator()), buy_date);
    CHECK_EQ(countBuy(sys), 1);
    CHECK_EQ(buyStoploss(sys), 0.0);
}

/** @arg a cloned system's stoploss gets the cloned system's own TM after run, not the source's;
 * before run neither m_tm nor m_kdata is carried (SL11) */
TEST_CASE("test_Stoploss_system_clone_injects_own_tm") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sz000001").getKData(KQuery(0, 10));
    Datetime buy_date = k[0].datetime;
    auto sys = makeStoplossSys(k, ST_FixedPercent(0.01), buy_date);
    auto sys_clone = sys->clone();
    // Immediately after clone the ST must not carry the source system's TM or kdata.
    CHECK_UNARY(sys_clone->getST()->getTM() == nullptr);
    CHECK_UNARY(sys_clone->getST()->getTO().empty());
    // After the cloned system runs, readyForRun re-injects the cloned system's own TM.
    sys_clone->run(k);
    CHECK_UNARY(sys_clone->getST()->getTM() != nullptr);
    CHECK_UNARY(sys_clone->getST()->getTM() == sys_clone->getTM());
}

/** @arg an invalid stop-loss level does not block the entry: the trade opens without stop-loss,
 * while a valid level keeps its protection */
TEST_CASE("test_Stoploss_invalid_level_opens_without_stop") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh600000");
    KData kdata = stock.getKData(KQueryByDate(Datetime(200006270000LL), Datetime(200007100000LL),
                                              KQuery::DAY, KQuery::FORWARD));
    CHECK_UNARY(!kdata.empty());
    Datetime buy_date(200006270000LL);

    /** @arg a negative stop-loss level is invalid, so the buy opens without stop-loss */
    auto bad = make_shared<StoplossInvalidReturn>();
    bad->setValue(-1.0);
    auto sys_negative = makeStoplossSys(kdata, bad, buy_date);
    CHECK_EQ(countBuy(sys_negative), 1);
    CHECK_EQ(buyStoploss(sys_negative), 0.0);

    /** @arg a Null (NaN) stop-loss level is invalid, so the buy opens without stop-loss */
    bad = make_shared<StoplossInvalidReturn>();
    bad->setValue(Null<price_t>());
    auto sys_null = makeStoplossSys(kdata, bad, buy_date);
    CHECK_EQ(countBuy(sys_null), 1);
    CHECK_EQ(buyStoploss(sys_null), 0.0);

    /** @arg a valid stop-loss level keeps its protection */
    auto sys_valid = makeStoplossSys(kdata, ST_FixedPercent(0.03), buy_date);
    CHECK_EQ(countBuy(sys_valid), 1);
    CHECK_UNARY(buyStoploss(sys_valid) > 0.0);

    /** @arg an indicator stop-loss during its warm-up also opens the buy without stop-loss */
    auto sys_warmup = makeStoplossSys(kdata, ST_Indicator(MA(CLOSE(), 5)), buy_date);
    CHECK_EQ(countBuy(sys_warmup), 1);
    CHECK_EQ(buyStoploss(sys_warmup), 0.0);
}

/** @} */
