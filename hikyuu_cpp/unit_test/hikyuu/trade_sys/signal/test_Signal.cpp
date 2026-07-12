/*
 * test_Signal.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cycle.h>
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

/**
 * @defgroup test_Signal test_Signal
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_Signal") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");

    SignalPtr p(new SignalTest);
    SignalTest *p_src = (SignalTest *)p.get();

    SUBCASE("Basic operation") {
        /** @arg 基本操作 */
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

        /** @arg 克隆操作 */
        p_src->setX(10);
        SignalPtr p_clone = p->clone();
        CHECK_NE(p, p_clone);
        p_src = (SignalTest *)p_clone.get();
        CHECK_EQ(p_src->getX(), 10);
        CHECK_EQ(p_clone->shouldBuy(Datetime(200101010000)), true);
        CHECK_EQ(p_clone->shouldSell(Datetime(200101030000)), true);

        /** @arg 插入重复买入日期 */
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

        /** @arg 插入重复卖出日期 */
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

        /** @arg 插入买入日期已经存在卖出指示 */
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

        /** @arg 插入卖出日期已经存在买入指示 */
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


/** @par 检测点 */
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

    /** @arg clone 后周期边界必须完整复制，不能把 end 写成 start */
    SignalPtr c = p->clone();
    CHECK_NE(p, c);
    CHECK_EQ(c->getCycleStart(), t0);
    CHECK_EQ(c->getCycleEnd(), t1);
}

/** @par 检测点 */
TEST_CASE("test_Signal_clone_operator_behavior") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData k = stock.getKData(KQuery(-30));
    REQUIRE(k.size() >= 15);

    Datetime t0 = k[0].datetime;
    Datetime t1 = k[10].datetime;
    REQUIRE(t0 < t1);

    // 父信号：cycle=true，已 startCycle（模拟 Operator 父节点持有周期边界）
    SignalPtr parent(new SignalTest);
    parent->setParam<bool>("cycle", true);
    parent->setTO(k);
    parent->startCycle(t0, t1);
    CHECK_EQ(parent->getCycleStart(), t0);
    CHECK_EQ(parent->getCycleEnd(), t1);

    // clone 后边界必须完整；修前 end 会变成 t0
    SignalPtr cloned = parent->clone();
    CHECK_EQ(cloned->getCycleStart(), t0);
    CHECK_EQ(cloned->getCycleEnd(), t1);

    // 模拟 OperatorSignal::sub_sg_calculate 的级联：
    //   child->startCycle(parent.m_cycle_start, parent.m_cycle_end)
    // 修前：startCycle(t0, t0) 触发 HKU_CHECK(start < close)
    // 修后：startCycle(t0, t1) 合法通过并产生买入信号
    auto child = SG_Cycle();
    child->setTO(k);
    REQUIRE_NOTHROW(child->startCycle(cloned->getCycleStart(), cloned->getCycleEnd()));
    CHECK_EQ(child->getCycleStart(), t0);
    CHECK_EQ(child->getCycleEnd(), t1);
    CHECK_EQ(child->getBuyValue(t0), 1.0);
}

/** @} */
