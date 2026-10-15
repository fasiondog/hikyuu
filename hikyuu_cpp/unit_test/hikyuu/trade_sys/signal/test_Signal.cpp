/*
 * test_Signal.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
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

        /** @arg Insert a buy date that already has a sell indication */
        p->reset();
        p->setParam<bool>("alternate", false);
        p->_addSellSignal(Datetime(200202010000));
        REQUIRE(p->shouldSell(Datetime(200202010000)));
        p->_addBuySignal(Datetime(200202010000));
        CHECK_UNARY(!p->shouldBuy(Datetime(200202010000)));

        p->reset();
        p->setParam<bool>("alternate", true);
        p->_addBuySignal(Datetime(200201010000));
        p->_addSellSignal(Datetime(200202010000));
        REQUIRE(p->shouldSell(Datetime(200202010000)));
        p->_addBuySignal(Datetime(200202010000));
        CHECK_UNARY(!p->shouldBuy(Datetime(200202010000)));

        /** @arg Insert a sell date that already has a buy indication */
        p->reset();
        p->setParam<bool>("alternate", false);
        p->_addBuySignal(Datetime(200202010000));
        p->_addSellSignal(Datetime(200202010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200202010000)));
        CHECK_UNARY(!p->shouldSell(Datetime(200202010000)));

        p->reset();
        p->setParam<bool>("alternate", true);
        p->_addBuySignal(Datetime(200202010000));
        p->_addSellSignal(Datetime(200202010000));
        CHECK_UNARY(p->shouldBuy(Datetime(200202010000)));
        CHECK_UNARY(!p->shouldSell(Datetime(200202010000)));
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
TEST_CASE("test_Signal_clone_cycle_bounds") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData k = stock.getKData(KQuery(-30));
    REQUIRE(k.size() >= 15);

    SignalPtr p(new SignalTest);
    p->setParam<bool>("cycle", true);
    p->setTO(k);

    Datetime t0 = k[0].datetime;
    Datetime t1 = k[10].datetime;
    REQUIRE(t0 < t1);
    p->startCycle(t0, t1);

    CHECK_EQ(p->getCycleStart(), t0);
    CHECK_EQ(p->getCycleEnd(), t1);

    /** @arg After clone the cycle boundary must be copied completely, end must not become start */
    SignalPtr c = p->clone();
    CHECK_NE(p, c);
    CHECK_EQ(c->getCycleStart(), t0);
    CHECK_EQ(c->getCycleEnd(), t1);
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
    parent->setParam<bool>("cycle", true);
    parent->setTO(k);
    parent->startCycle(t0, t1);
    CHECK_EQ(parent->getCycleStart(), t0);
    CHECK_EQ(parent->getCycleEnd(), t1);

    // The boundary must be complete after clone; before the fix end became t0
    SignalPtr cloned = parent->clone();
    CHECK_EQ(cloned->getCycleStart(), t0);
    CHECK_EQ(cloned->getCycleEnd(), t1);

    // Simulate the cascading of OperatorSignal::sub_sg_calculate:
    //   child->startCycle(parent.m_cycle_start, parent.m_cycle_end)
    // Before the fix: startCycle(t0, t0) triggers HKU_CHECK(start < close)
    // After the fix: startCycle(t0, t1) passes legally and produces a buy signal
    auto child = SG_Cycle();
    child->setTO(k);
    REQUIRE_NOTHROW(child->startCycle(cloned->getCycleStart(), cloned->getCycleEnd()));
    CHECK_EQ(child->getCycleStart(), t0);
    CHECK_EQ(child->getCycleEnd(), t1);
    CHECK_EQ(child->getBuyValue(t0), 1.0);
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

    Datetime first = k.front().datetime;
    Datetime last = k.back().datetime;

    /** @arg a cycle sub-signal in a normal combined signal does not throw and stays silent */
    SignalPtr or_sg = SG_Cycle() | make_shared<SignalCalcTest>();
    or_sg->setTO(k);
    CHECK_UNARY(or_sg->getBuySignal().size() == 1 && or_sg->getBuySignal()[0] == first);
    CHECK_UNARY(or_sg->getSellSignal().size() == 1 && or_sg->getSellSignal()[0] == last);

    /** @arg the cycle sub-signals are driven by startCycle */
    SignalPtr or_cycle = SG_Cycle() | SG_Cycle();
    or_cycle->setParam<bool>("cycle", true);
    or_cycle->setTO(k);
    Datetime start = k[5].datetime;
    Datetime close = k[10].datetime;
    or_cycle->startCycle(start, close);
    CHECK_UNARY(or_cycle->getBuySignal().size() == 1 && or_cycle->getBuySignal()[0] == start);
    CHECK_UNARY(or_cycle->getBuyValue(start) == 1.0);
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

/** @} */
