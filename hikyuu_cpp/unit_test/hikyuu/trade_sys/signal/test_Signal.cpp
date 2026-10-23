/*
 * test_Signal.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <chrono>
#include <limits>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/signal/crt/SG_Bool.h>
#include <hikyuu/trade_sys/signal/crt/SG_Band.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/signal/crt/SG_CrossGold.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cycle.h>
#include <hikyuu/trade_sys/signal/crt/SG_Single.h>
#include <hikyuu/trade_sys/signal/crt/SG_Logic.h>
#include <hikyuu/trade_sys/signal/crt/SG_Manual.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/indicator/crt/CVAL.h>
#include <hikyuu/indicator/crt/PRICELIST.h>
#include <hikyuu/KData.h>

using namespace hku;

class SignalTest : public SignalBase {
public:
    SignalTest() : SignalBase(), m_x(0) {}
    virtual ~SignalTest() {}

    void setX(int x) {
        m_x = x;
    }
    int getX() const {
        return m_x;
    }

    virtual void _reset() override {
        m_x = 0;
    }

    virtual SignalPtr _clone() override {
        SignalTest *p = new SignalTest;
        p->m_x = m_x;
        return SignalPtr(p);
    }

    virtual void _calculate(const KData &) override {}

private:
    int m_x;
};

class SignalCalcTest : public SignalBase {
public:
    SignalCalcTest() : SignalBase("SignalCalcTest") {}
    virtual ~SignalCalcTest() {}

    virtual void _calculate(const KData &kdata) override {
        m_calc_count++;
        _addBuySignal(kdata.front().datetime);
        _addSellSignal(kdata.back().datetime);
    }

    virtual SignalPtr _clone() override {
        return SignalPtr(new SignalCalcTest);
    }

    bool holdLong() const {
        return m_hold_long;
    }

    void setHoldLong(bool hold) {
        m_hold_long = hold;
    }

    int calcCount() const {
        return m_calc_count;
    }

private:
    int m_calc_count{0};
};

// A leaf indicator whose result length does not follow the KData context, used to build the
// ind.size() != kdata.size() input
class FixedLenInd : public IndicatorImp {
public:
    FixedLenInd(size_t len) : IndicatorImp("FIXEDLEN", 1), m_len(len) {}

    virtual void _calculate(const Indicator &) override {
        _readyBuffer(m_len, 1);
        for (size_t i = 0; i < m_len; ++i) {
            _set(1.0, i, 0);
        }
        m_discard = 0;
    }

    virtual IndicatorImpPtr _clone() override {
        return make_shared<FixedLenInd>(m_len);
    }

private:
    size_t m_len;
};

class BuyAtFirstSig : public SignalBase {
public:
    BuyAtFirstSig() : SignalBase("BuyAtFirstSig") {}
    virtual ~BuyAtFirstSig() {}

    virtual void _calculate(const KData &kdata) override {
        _addBuySignal(kdata.front().datetime);
    }

    virtual SignalPtr _clone() override {
        return SignalPtr(new BuyAtFirstSig);
    }
};

class SellAtFirstSig : public SignalBase {
public:
    SellAtFirstSig() : SignalBase("SellAtFirstSig") {}
    virtual ~SellAtFirstSig() {}

    virtual void _calculate(const KData &kdata) override {
        _addSellSignal(kdata.front().datetime);
    }

    virtual SignalPtr _clone() override {
        return SignalPtr(new SellAtFirstSig);
    }
};

/**
 * @defgroup test_Signal test_Signal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Signal") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");

    SignalPtr p(new SignalTest);
    SignalTest *p_src = (SignalTest *)p.get();

    SUBCASE("Basic operation") {
        /** @arg The basic operation */
        CHECK_EQ(p_src->getX(), 0);
        CHECK_EQ(p->name(), "SignalBase");
        p->name("SignalTest");
        CHECK_EQ(p->name(), "SignalTest");
        CHECK_EQ(p->shouldBuy(Datetime(200101010000)), false);
        p->_addBuySignal(Datetime(200101010000));
        CHECK_EQ(p->shouldBuy(Datetime(200101010000)), true);
        CHECK_EQ(p->getBuyValue(Datetime(200101010000)), 1.0);
        CHECK_EQ(p->shouldSell(Datetime(200101030000)), false);
        CHECK_EQ(p->getSellValue(Datetime(200101030000)), 0.0);
        p->_addSellSignal(Datetime(200101030000));
        CHECK_EQ(p->shouldSell(Datetime(200101030000)), true);

        /** @arg The clone operation */
        p_src->setX(10);
        SignalPtr p_clone = p->clone();
        CHECK_NE(p, p_clone);
        p_src = (SignalTest *)p_clone.get();
        CHECK_EQ(p_src->getX(), 10);
        CHECK_EQ(p_clone->shouldBuy(Datetime(200101010000)), true);
        CHECK_EQ(p_clone->shouldSell(Datetime(200101030000)), true);

        /** @arg Insert a duplicate buy date */
        p->reset();
        p->setParam<bool>("alternate", true);
        REQUIRE(!p->shouldBuy(Datetime(200201010000)));
        p->_addBuySignal(Datetime(200201010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200201010000)));
        p->_addBuySignal(Datetime(200201010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200201010000)));

        p->reset();
        p->setParam<bool>("alternate", false);
        REQUIRE(!p->shouldBuy(Datetime(200201010000)));
        p->_addBuySignal(Datetime(200201010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200201010000)));
        p->_addBuySignal(Datetime(200201010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200201010000)));

        /** @arg Insert a duplicate sell date */
        p->reset();
        p->setParam<bool>("alternate", true);
        p->_addBuySignal(Datetime(200201010000));
        REQUIRE(!p->shouldSell(Datetime(200201020000)));
        p->_addSellSignal(Datetime(200201020000));
        CHECK_UNARY(p->shouldSell(Datetime(200201020000)));
        p->_addSellSignal(Datetime(200201020000));
        CHECK_UNARY(p->shouldSell(Datetime(200201020000)));

        p->reset();
        p->setParam<bool>("alternate", false);
        REQUIRE(!p->shouldSell(Datetime(200201020000)));
        p->_addSellSignal(Datetime(200201020000));
        CHECK_UNARY(p->shouldSell(Datetime(200201020000)));
        p->_addSellSignal(Datetime(200201020000));
        CHECK_UNARY(p->shouldSell(Datetime(200201020000)));

        /** @arg A buy that nets the day to zero drops the sell record too */
        p->reset();
        p->setParam<bool>("alternate", false);
        p->_addSellSignal(Datetime(200202010000));
        REQUIRE(p->shouldSell(Datetime(200202010000)));
        p->_addBuySignal(Datetime(200202010000));
        CHECK_UNARY(!p->shouldBuy(Datetime(200202010000)));
        CHECK_UNARY(!p->shouldSell(Datetime(200202010000)));
        CHECK_EQ(p->getValue(Datetime(200202010000)), 0.0);

        p->reset();
        p->setParam<bool>("alternate", true);
        p->_addBuySignal(Datetime(200201010000));
        p->_addSellSignal(Datetime(200202010000));
        REQUIRE(p->shouldSell(Datetime(200202010000)));
        p->_addBuySignal(Datetime(200202010000));
        CHECK_UNARY(!p->shouldBuy(Datetime(200202010000)));
        CHECK_UNARY(!p->shouldSell(Datetime(200202010000)));

        /** @arg A sell that nets the day to zero drops the buy record too */
        p->reset();
        p->setParam<bool>("alternate", false);
        p->_addBuySignal(Datetime(200202010000));
        p->_addSellSignal(Datetime(200202010000));
        CHECK_UNARY(!p->shouldBuy(Datetime(200202010000)));
        CHECK_UNARY(!p->shouldSell(Datetime(200202010000)));
        CHECK_EQ(p->getValue(Datetime(200202010000)), 0.0);

        p->reset();
        p->setParam<bool>("alternate", true);
        p->_addBuySignal(Datetime(200202010000));
        p->_addSellSignal(Datetime(200202010000));
        CHECK_UNARY(!p->shouldBuy(Datetime(200202010000)));
        CHECK_UNARY(!p->shouldSell(Datetime(200202010000)));
        CHECK_EQ(p->getValue(Datetime(200202010000)), 0.0);
    }

    SUBCASE("Short sell") {
        p->name("SignalTest");
        p->setParam<bool>("support_borrow_stock", true);
        p->_addSellSignal(Datetime(200101010000));
        p->_addSellSignal(Datetime(200101030000));
        CHECK_EQ(p->shouldSell(Datetime(200101010000)), true);
        CHECK_EQ(p->shouldSell(Datetime(200101030000)), false);

        p->reset();
        p->setParam<bool>("support_borrow_stock", false);
        p->_addSellSignal(Datetime(200101010000));
        p->_addBuySignal(Datetime(200101030000));
        p->_addSellSignal(Datetime(200101040000));
        CHECK_EQ(p->shouldSell(Datetime(200101010000)), false);
        CHECK_EQ(p->shouldSell(Datetime(200101040000)), true);
    }
}

/** @par Test points */
TEST_CASE("test_Signal_same_bar_single_direction") {
    SignalPtr p(new SignalTest);
    Datetime d(200202010000);

    /** @arg a positive net drops the same-day sell record and keeps the net value */
    p->setParam<bool>("alternate", false);
    p->_addSellSignal(d, -1.0);
    REQUIRE_UNARY(p->shouldSell(d));
    p->_addBuySignal(d, 3.0);
    CHECK_UNARY(p->shouldBuy(d));
    CHECK_UNARY(!p->shouldSell(d));
    CHECK_EQ(p->getValue(d), 2.0);

    /** @arg a negative net drops the same-day buy record and keeps the net value */
    p->reset();
    p->_addBuySignal(d, 2.0);
    REQUIRE_UNARY(p->shouldBuy(d));
    p->_addSellSignal(d, -3.0);
    CHECK_UNARY(!p->shouldBuy(d));
    CHECK_UNARY(p->shouldSell(d));
    CHECK_EQ(p->getValue(d), -1.0);

    /** @arg a repeated same-direction input keeps the net value instead of double counting */
    p->reset();
    p->_addBuySignal(d, 1.0);
    p->_addBuySignal(d, 1.0);
    CHECK_EQ(p->getBuyValue(d), 2.0);

    /** @arg an alternate short entry flips to a buy when the net turns positive */
    p->reset();
    p->setParam<bool>("alternate", true);
    p->setParam<bool>("support_borrow_stock", true);
    p->_addSellSignal(d, -1.0);
    REQUIRE_UNARY(p->shouldSell(d));
    p->_addBuySignal(d, 3.0);
    CHECK_UNARY(p->shouldBuy(d));
    CHECK_UNARY(!p->shouldSell(d));
    CHECK_EQ(p->getValue(d), 2.0);

    /** @arg a zero net leaves no signal on that day in both alternation modes */
    for (bool alternate : {false, true}) {
        p->reset();
        p->setParam<bool>("alternate", alternate);
        p->setParam<bool>("support_borrow_stock", false);
        p->_addBuySignal(d, 1.0);
        p->_addSellSignal(d, -1.0);
        CHECK_UNARY(!p->shouldBuy(d));
        CHECK_UNARY(!p->shouldSell(d));
        CHECK_EQ(p->getValue(d), 0.0);
    }

    /** @arg the alternate state does not drop a same-day value refresh */
    p->reset();
    p->setParam<bool>("alternate", true);
    p->_addBuySignal(d, 2.0);
    p->_addBuySignal(d, 1.0);
    CHECK_EQ(p->getBuyValue(d), 3.0);

    p->reset();
    p->_addBuySignal(d, 2.0);
    p->_addSellSignal(d, -1.0);
    CHECK_UNARY(p->shouldBuy(d));
    CHECK_UNARY(!p->shouldSell(d));
    CHECK_EQ(p->getBuyValue(d), 1.0);
}

/** @par Test points */
TEST_CASE("test_Signal_clone_cycle_bounds") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData k = stock.getKData(KQuery(-30));
    REQUIRE(k.size() >= 15);

    SignalPtr p(new SignalTest);
    p->setParam<bool>("alternate", false);
    p->setParam<bool>("cycle", true);
    p->setTO(k);

    Datetime t0 = k[0].datetime;
    Datetime t1 = k[10].datetime;
    REQUIRE(t0 < t1);
    p->startCycle(t0, t1);

    CHECK_EQ(p->getCycleStart(), t0);
    CHECK_EQ(p->getCycleEnd(), t1);

    /** @arg the cycle start is registered into the declared rebalancing days */
    CHECK_UNARY(p->getCycleDates().size() == 1 && p->getCycleDates()[0] == t0);

    /** @arg After clone the cycle boundary must be copied completely, end must not become start */
    SignalPtr c = p->clone();
    CHECK_NE(p, c);
    CHECK_EQ(c->getCycleStart(), t0);
    CHECK_EQ(c->getCycleEnd(), t1);
    CHECK_UNARY(c->getCycleDates() == p->getCycleDates());
}

/** @par Test points */
TEST_CASE("test_Signal_clone_operator_behavior") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData k = stock.getKData(KQuery(-30));
    REQUIRE(k.size() >= 15);

    Datetime t0 = k[0].datetime;
    Datetime t1 = k[10].datetime;
    REQUIRE(t0 < t1);

    // The parent signal: cycle=true and startCycle already called (an Operator parent holds it)
    SignalPtr parent(new SignalTest);
    parent->setParam<bool>("alternate", false);
    parent->setParam<bool>("cycle", true);
    parent->setTO(k);
    parent->startCycle(t0, t1);
    CHECK_EQ(parent->getCycleStart(), t0);
    CHECK_EQ(parent->getCycleEnd(), t1);

    // The boundary must be complete after clone; before the fix end became t0
    SignalPtr cloned = parent->clone();
    CHECK_EQ(cloned->getCycleStart(), t0);
    CHECK_EQ(cloned->getCycleEnd(), t1);

    // Simulate the cycle start declaration on a child: startCycle registers the day and
    // recalculates the whole trading object once (no window slicing)
    auto child = SG_Cycle();
    child->setTO(k);
    REQUIRE_NOTHROW(child->startCycle(cloned->getCycleStart(), cloned->getCycleEnd()));
    CHECK_EQ(child->getCycleStart(), t0);
    CHECK_EQ(child->getCycleEnd(), t1);
    CHECK_EQ(child->getBuyValue(t0), 1.0);
}

/** @par Test points */
TEST_CASE("test_Signal_clone_lazy_recalculate") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData k = stock.getKData(KQuery(-30));
    REQUIRE(k.size() >= 15);

    auto p = make_shared<SignalCalcTest>();
    p->setTO(k);
    Datetime t0 = k[0].datetime;
    Datetime tN = k[k.size() - 1].datetime;
    CHECK_EQ(p->getBuyValue(t0), 1.0);
    CHECK_EQ(p->getSellValue(tN), -1.0);

    /** @arg the clone keeps the copied signal table, so queries work before any recalculation */
    SignalPtr c = p->clone();
    CHECK_NE(p, c);
    CHECK_EQ(c->getBuyValue(t0), 1.0);
    CHECK_EQ(c->getSellValue(tN), -1.0);

    /** @arg the clone recalculates lazily: setTO with the same KData must re-run _calculate
     * instead of short-circuiting, so the subclass runtime state is rebuilt */
    auto *cloned = static_cast<SignalCalcTest *>(c.get());
    CHECK_EQ(cloned->calcCount(), 0);
    c->setTO(k);
    CHECK_EQ(cloned->calcCount(), 1);
    CHECK_EQ(cloned->getBuyValue(t0), 1.0);
    CHECK_EQ(cloned->getSellValue(tN), -1.0);
    CHECK_EQ(static_cast<SignalCalcTest *>(p.get())->calcCount(), 1);
}

/** @par Test points */
TEST_CASE("test_Signal_setTO_recalculate") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    REQUIRE_UNARY(!stock.isNull());

    auto sg = make_shared<SignalCalcTest>();
    KData k1 = stock.getKData(KQueryByIndex(10, 30));
    KData k2 = stock.getKData(KQueryByIndex(10, 31));
    REQUIRE_UNARY(!k1.empty() && !k2.empty());
    Datetime d10 = k1[0].datetime;
    Datetime d29 = k1.back().datetime;

    sg->setTO(k1);
    CHECK_UNARY(sg->shouldBuy(d10));
    CHECK_UNARY(sg->shouldSell(d29));

    /** @arg The hold flags are reset before the recalculation (ISS-027) */
    sg->setHoldLong(true);

    /** @arg Recalculating on a changed KData clears the stale signals: the overlapping date value
     * is not accumulated and the old window signals disappear (ISS-027) */
    sg->setTO(k2);
    CHECK_EQ(sg->getBuyValue(d10), doctest::Approx(1.0));
    CHECK_UNARY(!sg->shouldSell(d29));
    CHECK_UNARY(sg->shouldSell(k2.back().datetime));
    CHECK_UNARY(sg->shouldBuy(k2.front().datetime));
    CHECK_UNARY(!sg->holdLong());
}

/** @par Test points */
TEST_CASE("test_SG_Bool_non_finite") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 10));
    if (k.empty()) {
        return;
    }

    /** @arg a non-finite buy series does not fire any signal */
    SignalPtr sg = SG_Bool(CVAL(std::numeric_limits<double>::infinity()), CVAL(0.0));
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());
}

/** @par Test points */
TEST_CASE("test_SG_Band_non_finite") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 20));
    if (k.empty()) {
        return;
    }

    /** @arg a non-finite price-band input fires no signal (the inf bar would exceed the band) */
    SignalPtr sg = SG_Band(CVAL(std::numeric_limits<double>::infinity()), -1.0, 1.0);
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());

    /** @arg the indicator-band overload behaves the same */
    SignalPtr sg2 = SG_Band(CVAL(std::numeric_limits<double>::infinity()), CVAL(-1.0), CVAL(1.0));
    sg2->setTO(k);
    CHECK_UNARY(sg2->getBuySignal().empty());
    CHECK_UNARY(sg2->getSellSignal().empty());
}

/** @par Test points */
TEST_CASE("test_SG_Single_sell_threshold") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    DatetimeList dates = k.getDatetimeList();
    PriceList vals(k.size(), 10.98);
    vals[0] = 10.0;
    vals[1] = 10.2;
    vals[2] = 10.4;
    vals[3] = 10.6;
    vals[4] = 10.8;
    vals[5] = 11.5;
    vals[6] = 11.48;
    vals[7] = 10.98;
    SignalPtr sg = SG_Single(PRICELIST(vals, dates), 5, 0.5);
    sg->setTO(k);

    /** @arg a large rise exceeding the noise threshold fires a buy signal */
    DatetimeList buy = sg->getBuySignal();
    CHECK_UNARY(buy.size() == 1 && buy[0] == dates[5]);

    /** @arg a small decline below the noise threshold fires no sell signal */
    DatetimeList sell = sg->getSellSignal();
    CHECK_UNARY(sell.size() == 1 && sell[0] == dates[7]);

    /** @arg a large decline exceeding the noise threshold fires a sell signal */
    CHECK_UNARY(sell.size() == 1 && sell[0] == dates[7]);
}

/** @par Test points */
TEST_CASE("test_SG_OperatorValue_clone_mode") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    Datetime first = k.front().datetime;
    Datetime last = k.back().datetime;
    SignalPtr base = make_shared<SignalCalcTest>();

    /** @arg the clone of (sg - 2.0) keeps mode 0 and yields the same signals */
    SignalPtr sub0 = base - 2.0;
    sub0->setParam<bool>("alternate", false);
    sub0->setTO(k);
    SignalPtr sub0_clone = sub0->clone();
    sub0_clone->setTO(k);
    CHECK_UNARY(sub0_clone->getBuySignal().empty());
    CHECK_UNARY(sub0_clone->getSellSignal().size() == 2 &&
                sub0_clone->getSellSignal()[0] == first && sub0_clone->getSellSignal()[1] == last);
    CHECK_UNARY(sub0_clone->getSellValue(first) == sub0->getSellValue(first));
    CHECK_UNARY(sub0_clone->getSellValue(last) == sub0->getSellValue(last));

    /** @arg the clone of (2.0 - sg) keeps mode 1 and does not flip the buy/sell direction */
    SignalPtr sub1 = 2.0 - base;
    sub1->setParam<bool>("alternate", false);
    sub1->setTO(k);
    SignalPtr sub1_clone = sub1->clone();
    sub1_clone->setTO(k);
    CHECK_UNARY(sub1_clone->getSellSignal().empty());
    CHECK_UNARY(sub1_clone->getBuySignal().size() == 2 && sub1_clone->getBuySignal()[0] == first &&
                sub1_clone->getBuySignal()[1] == last);
    CHECK_UNARY(sub1_clone->getBuyValue(first) == sub1->getBuyValue(first));
    CHECK_UNARY(sub1_clone->getBuyValue(last) == sub1->getBuyValue(last));

    /** @arg the clone of (2.0 / sg) keeps mode 1 and yields the same signal values */
    SignalPtr div1 = 2.0 / base;
    div1->setParam<bool>("alternate", false);
    div1->setTO(k);
    SignalPtr div1_clone = div1->clone();
    div1_clone->setTO(k);
    CHECK_UNARY(div1_clone->getBuyValue(first) == div1->getBuyValue(first));
    CHECK_UNARY(div1_clone->getSellValue(last) == div1->getSellValue(last));
    CHECK_UNARY(div1_clone->getBuyValue(first) == 2.0);
    CHECK_UNARY(div1_clone->getSellValue(last) == -2.0);
}

/** @par Test points */
TEST_CASE("test_SG_Operator_cycle_sub") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    Datetime start = k[5].datetime;
    Datetime start2 = k[8].datetime;

    /** @arg a combined signal with one cycle child inherits both the cycle attribute and the
     * non-alternating behavior of the scheduled entry; the declared days are forwarded to the
     * children and take part in the single full-range calculation, while the normal child keeps
     * its own full-range signals */
    SignalPtr or_mixed = SG_Cycle() | make_shared<SignalCalcTest>();
    CHECK_UNARY(or_mixed->getParam<bool>("cycle"));
    CHECK_UNARY(!or_mixed->getParam<bool>("alternate"));
    or_mixed->setCycleDates({start, start2});
    or_mixed->setTO(k);
    CHECK_UNARY(or_mixed->getBuySignal().size() == 3);
    CHECK_EQ(or_mixed->getBuyValue(start), 1.0);
    CHECK_EQ(or_mixed->getBuyValue(start2), 1.0);
    CHECK_EQ(or_mixed->getBuyValue(k.front().datetime), 1.0);
    CHECK_UNARY(or_mixed->getSellSignal().size() == 1 &&
                or_mixed->getSellSignal()[0] == k.back().datetime);

    /** @arg a combined signal whose both children are cycle-driven consumes the same declaration,
     * every declared day yields its buy */
    SignalPtr or_cycle = SG_Cycle() | SG_Cycle();
    CHECK_UNARY(or_cycle->getParam<bool>("cycle"));
    CHECK_UNARY(!or_cycle->getParam<bool>("alternate"));
    or_cycle->setCycleDates({start, start2});
    or_cycle->setTO(k);
    CHECK_UNARY(or_cycle->getBuySignal().size() == 2);
    CHECK_EQ(or_cycle->getBuyValue(start), 1.0);
    CHECK_EQ(or_cycle->getBuyValue(start2), 1.0);
    CHECK_UNARY(or_cycle->getSellSignal().empty());
}

/** @par Test points */
TEST_CASE("test_SG_ValueSignal_cycle_sub") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    /** @arg a cycle sub-signal in a value combined signal consumes the declared days and keeps the
     * non-alternating behavior of the scheduled entry */
    SignalPtr mul = SG_Cycle() * 2.0;
    Datetime start = k[5].datetime;
    Datetime start2 = k[8].datetime;
    CHECK_UNARY(!mul->getParam<bool>("alternate"));
    mul->setCycleDates({start, start2});
    mul->setTO(k);
    CHECK_UNARY(mul->getBuySignal().size() == 2);
    CHECK_UNARY(mul->getBuyValue(start) == 2.0);
    CHECK_UNARY(mul->getBuyValue(start2) == 2.0);
}

/** @par Test points */
TEST_CASE("test_Signal_cycle_flag_not_change_calculation") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 40));
    if (k.size() < 30) {
        return;
    }

    DatetimeList dates = k.getDatetimeList();
    PriceList fast_vals(k.size(), 0.0);
    PriceList slow_vals(k.size(), 0.5);
    for (size_t i = 0; i < k.size(); ++i) {
        fast_vals[i] = (i % 6 < 3) ? 1.0 : 0.0;
    }

    /** @arg the cycle flag no longer changes the calculation range: the same indicator signal
     * yields the same table with cycle=true and cycle=false (both non-alternating, so that the
     * comparison isolates the calculation range) */
    SignalPtr normal = SG_Cross(PRICELIST(fast_vals, dates), PRICELIST(slow_vals, dates));
    normal->setParam<bool>("alternate", false);
    normal->setTO(k);
    CHECK_UNARY(!normal->getBuySignal().empty() || !normal->getSellSignal().empty());

    SignalPtr cycle = SG_Cross(PRICELIST(fast_vals, dates), PRICELIST(slow_vals, dates));
    cycle->setParam<bool>("cycle", true);
    CHECK_UNARY(!cycle->getParam<bool>("alternate"));
    cycle->setTO(k);
    CHECK_UNARY(cycle->getBuySignal() == normal->getBuySignal());
    CHECK_UNARY(cycle->getSellSignal() == normal->getSellSignal());
}

/** @par Test points */
TEST_CASE("test_Signal_cycle_forces_no_alternate") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    /** @arg a cycle signal is a scheduled entry: alternate=true is coerced back */
    auto p = make_shared<SignalCalcTest>();
    p->setParam<bool>("alternate", false);
    p->setParam<bool>("cycle", true);
    CHECK_UNARY(!p->getParam<bool>("alternate"));
    p->setParam<bool>("alternate", true);
    CHECK_UNARY(!p->getParam<bool>("alternate"));

    /** @arg SG_Cycle follows the same invariant (no hard exception left behind) */
    SignalPtr sc = SG_Cycle();
    sc->setParam<bool>("alternate", true);
    CHECK_UNARY(!sc->getParam<bool>("alternate"));

    /** @arg the same invariant holds when switching an already alternate signal to cycle */
    auto q = make_shared<SignalCalcTest>();
    CHECK_UNARY(q->getParam<bool>("alternate"));
    q->setParam<bool>("cycle", true);
    CHECK_UNARY(!q->getParam<bool>("alternate"));

    /** @arg a composition containing a cycle child keeps the invariant even if alternate is set
     * explicitly afterwards */
    SignalPtr comp = SG_Cycle() | SG_Cycle();
    CHECK_UNARY(!comp->getParam<bool>("alternate"));
    comp->setParam<bool>("alternate", true);
    CHECK_UNARY(!comp->getParam<bool>("alternate"));
}

/** @par Test points */
TEST_CASE("test_SG_Cycle_injected_dates") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    Datetime t0 = k[5].datetime;
    Datetime t1 = k[20].datetime;

    /** @arg a cycle signal stays silent without the declared rebalancing days */
    SignalPtr sg = SG_Cycle();
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());

    /** @arg the declared days inside the trading object are emitted, the outside ones ignored */
    sg->setCycleDates({t0, t1, k.back().datetime + Days(1)});
    CHECK_UNARY(sg->getBuySignal().size() == 2);
    CHECK_EQ(sg->getBuyValue(t0), 1.0);
    CHECK_EQ(sg->getBuyValue(t1), 1.0);

    /** @arg startCycle registers one more day of the same list */
    Datetime t2 = k[10].datetime;
    sg->startCycle(t2, k[15].datetime);
    CHECK_UNARY(sg->getBuySignal().size() == 3);
    CHECK_EQ(sg->getBuyValue(t2), 1.0);
}

/** @par Test points */
TEST_CASE("test_Signal_set_cycle_dates_recalculate_once") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    auto p = make_shared<SignalCalcTest>();
    p->setParam<bool>("cycle", true);
    p->setTO(k);
    CHECK_EQ(p->calcCount(), 1);

    /** @arg a changed declared list triggers exactly one full-range recalculation */
    p->setCycleDates({k[0].datetime, k[5].datetime});
    CHECK_EQ(p->calcCount(), 2);

    /** @arg the same list in another order is a no-op */
    p->setCycleDates({k[5].datetime, k[0].datetime});
    CHECK_EQ(p->calcCount(), 2);

    /** @arg startCycle registers the day through the same path: one more recalculation */
    p->startCycle(k[10].datetime, k[15].datetime);
    CHECK_EQ(p->calcCount(), 3);
    CHECK_EQ(p->getCycleDates().size(), 3);
}

/** @par Test points */
TEST_CASE("test_SG_Operator_sub_recalculated_on_new_kdata") {
    StockManager &sm = StockManager::instance();
    KData k1 = sm.getStock("sh000001").getKData(KQuery(0, 30));
    KData k2 = sm.getStock("sh000001").getKData(KQuery(10, 40));
    if (k1.size() < 25 || k2.size() < 25) {
        return;
    }

    /** @arg the sub-signals are cleared and recalculated when the KData changes */
    SignalPtr or_sg = make_shared<SignalCalcTest>() | make_shared<SignalCalcTest>();
    or_sg->setTO(k1);
    CHECK_UNARY(or_sg->getBuySignal().size() == 1 &&
                or_sg->getBuySignal()[0] == k1.front().datetime);

    or_sg->setTO(k2);
    CHECK_UNARY(or_sg->getBuySignal().size() == 1 &&
                or_sg->getBuySignal()[0] == k2.front().datetime);
    CHECK_UNARY(or_sg->getSellSignal().size() == 1 &&
                or_sg->getSellSignal()[0] == k2.back().datetime);
}

/** @par Test points */
TEST_CASE("test_SG_zero_residual_filtered") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    /** @arg a ~1e-16 float residual from value operations fires no signal */
    SignalPtr sg = make_shared<SignalCalcTest>();
    SignalPtr residual = (sg * 0.1) + (sg * 0.2) - (sg * 0.3);
    residual->setTO(k);
    CHECK_UNARY(residual->getBuySignal().empty());
    CHECK_UNARY(residual->getSellSignal().empty());

    /** @arg real values above the zero tolerance still fire */
    SignalPtr exact = (sg * 0.1) + (sg * 0.2) - (sg * 0.2);
    exact->setTO(k);
    CHECK_UNARY(exact->getBuySignal().size() == 1);
    CHECK_UNARY(exact->getSellSignal().size() == 1);
}

/** @par Test points */
TEST_CASE("test_SG_Or_same_bar_conflict_sell_first") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    /** @arg same-bar opposite signals: sell wins and no buy is emitted on the conflict bar */
    auto buy_sig = make_shared<BuyAtFirstSig>();
    auto sell_sig = make_shared<SellAtFirstSig>();
    buy_sig->setParam<bool>("alternate", false);
    sell_sig->setParam<bool>("alternate", false);
    auto conflict = buy_sig | sell_sig;
    conflict->setParam<bool>("alternate", false);
    conflict->setTO(k);
    CHECK_UNARY(conflict->getBuySignal().empty());
    CHECK_UNARY(conflict->getSellSignal().size() == 1);
    CHECK_UNARY(conflict->getSellSignal()[0] == k.front().datetime);
}

/** @par Test points */
TEST_CASE("test_SG_Bool_same_bar_conflict_sell_first") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    /** @arg buy and sell both true on every bar: only sell signals are recorded */
    auto sg = SG_Bool(CVAL(1.0), CVAL(1.0), false);
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().size() == k.size());

    /** @arg mutually exclusive conditions keep their own signals */
    auto sg2 = SG_Bool(CVAL(1.0), CVAL(0.0), false);
    sg2->setTO(k);
    CHECK_UNARY(sg2->getBuySignal().size() == k.size());
    CHECK_UNARY(sg2->getSellSignal().empty());
}

/** @par Test points */
TEST_CASE("test_SG_Cross_non_finite") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 20));
    if (k.empty()) {
        return;
    }

    /** @arg an inf point that would cross the slow line fires no signal */
    DatetimeList dates = k.getDatetimeList();
    PriceList fast_vals(k.size(), 0.0);
    fast_vals[1] = std::numeric_limits<double>::infinity();
    SignalPtr sg =
      SG_Cross(PRICELIST(fast_vals, dates), PRICELIST(PriceList(k.size(), 0.5), dates));
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());
}

/** @par Test points */
TEST_CASE("test_SG_CrossGold_non_finite") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 20));
    if (k.empty()) {
        return;
    }

    /** @arg an inf point that would satisfy the golden cross fires no signal */
    DatetimeList dates = k.getDatetimeList();
    PriceList fast_vals(k.size(), 0.0);
    fast_vals[1] = std::numeric_limits<double>::infinity();
    PriceList slow_vals(k.size(), 0.0);
    for (size_t i = 0; i < k.size(); ++i) {
        slow_vals[i] = 0.5 + 0.001 * static_cast<double>(i);
    }
    SignalPtr sg = SG_CrossGold(PRICELIST(fast_vals, dates), PRICELIST(slow_vals, dates));
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());
}

/** @par Test points */
TEST_CASE("test_SG_Single_non_finite") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    /** @arg an inf jump fires no signal */
    DatetimeList dates = k.getDatetimeList();
    PriceList vals(k.size(), 0.0);
    vals[20] = std::numeric_limits<double>::infinity();
    SignalPtr sg = SG_Single(PRICELIST(vals, dates), 10, 0.1);
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());
}

/** @par Test points */
TEST_CASE("test_SG_Single2_non_finite") {
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 30));
    if (k.size() < 25) {
        return;
    }

    /** @arg an inf jump fires no signal */
    DatetimeList dates = k.getDatetimeList();
    PriceList vals(k.size(), 0.0);
    vals[20] = std::numeric_limits<double>::infinity();
    SignalPtr sg = SG_Single2(PRICELIST(vals, dates), 10, 0.1);
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());
}

/** @par Test points */
TEST_CASE("test_SG_kdata_size_mismatch") {
    // Mismatched inputs are salvaged via ALIGN by dates; the aligned constant series fires no
    // signal, and a failed alignment (defensive) also yields an empty signal without OOB access
    StockManager &sm = StockManager::instance();
    KData k = sm.getStock("sh000001").getKData(KQuery(0, 20));
    if (k.empty()) {
        return;
    }

    /** @arg SG_Cross with an indicator longer than the KData fires no signal */
    SignalPtr sg = SG_Cross(Indicator(make_shared<FixedLenInd>(k.size() + 10)),
                            Indicator(make_shared<FixedLenInd>(k.size() + 10)));
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());

    /** @arg SG_CrossGold with an indicator shorter than the KData fires no signal */
    sg =
      SG_CrossGold(Indicator(make_shared<FixedLenInd>(5)), Indicator(make_shared<FixedLenInd>(5)));
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());

    /** @arg SG_Band with an indicator longer than the KData fires no signal */
    sg = SG_Band(Indicator(make_shared<FixedLenInd>(k.size() + 10)), -1.0, 1.0);
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());

    /** @arg SG_Band (indicator overload) with an indicator shorter than the KData fires no signal
     */
    sg = SG_Band(Indicator(make_shared<FixedLenInd>(5)), Indicator(make_shared<FixedLenInd>(5)),
                 Indicator(make_shared<FixedLenInd>(5)));
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());

    /** @arg SG_Single with an indicator longer than the KData fires no signal */
    sg = SG_Single(Indicator(make_shared<FixedLenInd>(k.size() + 10)), 10, 0.1);
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());

    /** @arg SG_Single2 with an indicator shorter than the KData fires no signal */
    sg = SG_Single2(Indicator(make_shared<FixedLenInd>(5)), 10, 0.1);
    sg->setTO(k);
    CHECK_UNARY(sg->getBuySignal().empty());
    CHECK_UNARY(sg->getSellSignal().empty());
}

// A step signal generator for the SG-008 operator benchmark: emits buy/sell signals at fixed bar
// steps so the logic operator _calculate cost can be measured against real KData
class BenchStepSig : public SignalBase {
public:
    BenchStepSig(size_t buy_step, size_t sell_step)
    : SignalBase("BenchStepSig"), m_buy_step(buy_step), m_sell_step(sell_step) {}
    virtual ~BenchStepSig() {}

    virtual SignalPtr _clone() override {
        return SignalPtr(new BenchStepSig(m_buy_step, m_sell_step));
    }

    virtual void _calculate(const KData &kdata) override {
        size_t total = kdata.size();
        for (size_t i = 0; i < total; ++i) {
            if (i % m_buy_step == 0) {
                _addBuySignal(kdata[i].datetime);
            }
            if (i % m_sell_step == 0) {
                _addSellSignal(kdata[i].datetime);
            }
        }
    }

private:
    size_t m_buy_step;
    size_t m_sell_step;
};

/** @par Test points (benchmark: prints the cost only, no time assertion) */
TEST_CASE("bench_SG_logic_operator") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0));
    if (kdata.size() < 100) {
        return;
    }
    std::cout << "bench_SG_logic_operator: kdata size: " << kdata.size() << std::endl;

    const int runs = 100;
    struct BenchCase {
        const char *density;
        size_t buy_step;
        size_t sell_step;
    };
    BenchCase cases[] = {
      {"dense", 3, 5},
      {"sparse", 20, 15},
    };

    for (const auto &bench_case : cases) {
        for (int op = 0; op < 3; ++op) {
            // Add represents the additive family (Add/Sub); Mul the multiplicative family
            // (Mul/Div/And); Or the branch family
            auto sg1 = make_shared<BenchStepSig>(bench_case.buy_step, bench_case.sell_step);
            auto sg2 = make_shared<BenchStepSig>(bench_case.buy_step, bench_case.sell_step);
            SignalPtr sg;
            const char *op_name = "Add";
            switch (op) {
                case 0:
                    sg = sg1 + sg2;
                    op_name = "Add";
                    break;
                case 1:
                    sg = sg1 * sg2;
                    op_name = "Mul";
                    break;
                case 2:
                    sg = sg1 | sg2;
                    op_name = "Or";
                    break;
            }
            REQUIRE_UNARY(sg);

            sg->setTO(kdata);
            size_t expect_buy = sg->getBuySignal().size();
            size_t expect_sell = sg->getSellSignal().size();
            CHECK_UNARY(expect_buy + expect_sell > 0);

            // warm: flip a no-op param to invalidate only the operator itself, keeping the
            // sub-signals warm, so the loop measures the operator _calculate alone
            auto t0 = std::chrono::steady_clock::now();
            for (int i = 0; i < runs; ++i) {
                sg->setParam<bool>("support_borrow_stock", false);
                sg->setTO(kdata);
            }
            auto t1 = std::chrono::steady_clock::now();
            double warm_ms =
              std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000.0;

            // cold: reset propagates to the sub-signals, so each run includes their recalculation
            t0 = std::chrono::steady_clock::now();
            for (int i = 0; i < runs; ++i) {
                sg->reset();
                sg->setTO(kdata);
            }
            t1 = std::chrono::steady_clock::now();
            double cold_ms =
              std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000.0;

            CHECK_UNARY(sg->getBuySignal().size() == expect_buy &&
                        sg->getSellSignal().size() == expect_sell);

            std::cout << "  [" << bench_case.density << "] " << op_name << " x" << runs
                      << " setTO: warm(operator only) " << warm_ms << " ms (" << warm_ms * 10.0
                      << " us/run), cold(with sub calc) " << cold_ms << " ms (" << cold_ms * 10.0
                      << " us/run)" << std::endl;
        }
    }
}

/** @par Test points */
TEST_CASE("test_SG_Manual_direct_drive") {
    KData kdata = getStock("sz000001").getKData(KQueryByIndex(-20));
    REQUIRE(kdata.size() > 3);

    auto sg = SG_Manual();
    sg->_addBuySignal(kdata[1].datetime);
    sg->_addSellSignal(kdata[3].datetime);

    /** @arg a new trading object keeps the seeded signals */
    sg->setTO(kdata);
    CHECK_EQ(sg->getBuySignal().size(), 1);
    CHECK_EQ(sg->getSellSignal().size(), 1);
    CHECK_UNARY(sg->shouldBuy(kdata[1].datetime));

    /** @arg reset keeps them, they are the input of a manual signal */
    sg->reset();
    CHECK_EQ(sg->getBuySignal().size(), 1);

    /** @arg as the SG of a system, whose reset and setTO both run */
    auto sys = SYS_Simple(crtTM(Datetime(200001010000LL), 100000.0), MM_Nothing(), EnvironmentPtr(),
                          ConditionPtr(), sg);
    sys->run(kdata);
    CHECK_UNARY(sys->getSG()->shouldBuy(kdata[1].datetime));
    CHECK_UNARY(sys->getSG()->shouldSell(kdata[3].datetime));
}

/** @par Test points */
TEST_CASE("test_SG_Cycle_no_bar_date") {
    // sh600005 has no bar on 2000-06-30 (halted), the calendar still carries the day
    Stock stk = getStock("sh600005");
    REQUIRE(!stk.isNull());
    KData kdata = stk.getKData(KQuery(Datetime(20000626), Datetime(20000710)));
    REQUIRE(kdata.size() > 1);
    const Datetime halted(20000630);

    bool has_halted_bar = false;
    Datetime next_bar;
    for (size_t i = 0; i < kdata.size(); i++) {
        if (kdata[i].datetime == halted) {
            has_halted_bar = true;
        } else if (kdata[i].datetime > halted && next_bar.isNull()) {
            next_bar = kdata[i].datetime;
        }
    }
    REQUIRE_UNARY(!has_halted_bar);
    REQUIRE_UNARY(!next_bar.isNull());

    auto sg = SG_Cycle();
    sg->setCycleDates(DatetimeList{halted});
    sg->setTO(kdata);

    /** @arg the day still holds the scheduled signal */
    CHECK_UNARY(sg->shouldBuy(halted));
    /** @arg it is not shifted to the first bar after the halt */
    CHECK_UNARY(!sg->shouldBuy(next_bar));
}

/** @} */
