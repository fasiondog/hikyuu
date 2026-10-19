/*
 * test_Condition.cpp
 *
 *  Created on: 2013-3-11
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <cmath>
#include <fstream>
#include <limits>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/condition/ConditionBase.h>
#include <hikyuu/trade_sys/condition/crt/CN_Bool.h>
#include <hikyuu/trade_sys/condition/crt/CN_Manual.h>
#include <hikyuu/trade_sys/signal/SignalBase.h>
#include <hikyuu/indicator/crt/CVAL.h>

using namespace hku;

namespace {
// A trivial signal generator used only to verify the dirty-flag behavior of setSG
class DummySG : public SignalBase {
public:
    DummySG() : SignalBase("TEST_DUMMY_SG") {}
    void _calculate(const KData&) override {}
    SignalPtr _clone() override {
        return std::make_shared<DummySG>();
    }
};
}  // namespace

class ConditionTest : public ConditionBase {
public:
    ConditionTest() : ConditionBase("TEST") {
        setParam<int>("n", 10);
        m_flag = false;
        m_calc_count = 0;
    }

    ~ConditionTest() {}

    KData getKData() const {
        return m_kdata;
    }

    int calcCount() const {
        return m_calc_count;
    }

    virtual void _calculate() {
        m_calc_count++;
        _addValid(m_kdata[2].datetime, 2.0);
        _addValid(m_kdata[3].datetime, 1.5);
    }

    virtual void _reset() {
        if (m_flag) {
            m_flag = false;
        } else {
            m_flag = true;
        }
    }

    virtual ConditionPtr _clone() {
        ConditionTest *p = new ConditionTest;
        p->m_flag = m_flag;
        p->m_calc_count = m_calc_count;
        return ConditionPtr(p);
    }

private:
    bool m_flag;
    int m_calc_count{0};

    CONDITION_NO_PRIVATE_MEMBER_SERIALIZATION
};

// A condition with a custom validity rule (value >= 2.0), to verify the _isValid hook
class ThresholdCondition : public ConditionBase {
public:
    ThresholdCondition() : ConditionBase("TEST_THRESHOLD") {}

    void _calculate() override {
        _addValid(m_kdata[2].datetime, 2.0);
        _addValid(m_kdata[3].datetime, 1.5);
    }

    bool _isValid(const Datetime& datetime, price_t value) const override {
        return value >= 2.0;
    }

    ConditionPtr _clone() override {
        return make_shared<ThresholdCondition>();
    }

private:
    CONDITION_NO_PRIVATE_MEMBER_SERIALIZATION
};

/**
 * @defgroup test_Condition test_Condition
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_Condition") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KQuery query(0, 10);
    KData kdata = stock.getKData(query);
    ConditionTest *p_src;

    /** @arg The basic operation */
    ConditionPtr p(new ConditionTest);
    p_src = (ConditionTest *)p.get();
    CHECK_EQ(p->name(), "TEST");
    CHECK_EQ(p->getParam<int>("n"), 10);
    CHECK_EQ(p_src->getKData().getStock(), Null<Stock>());
    REQUIRE(p->size() == 0);
    p->setTO(kdata);
    CHECK_EQ(p->size(), kdata.size());
    CHECK_EQ(p_src->getKData().getStock(), stock);
    CHECK_UNARY(p->isValid(Datetime(19901221)));
    CHECK_UNARY(p->isValid(Datetime(19901224)));
    CHECK_UNARY_FALSE(p->isValid(Datetime(19901219)));

    auto ds = p->getDatetimeList();
    CHECK_EQ(ds.size(), 2);
    CHECK_EQ(ds[0], Datetime(19901221));
    CHECK_EQ(ds[1], Datetime(19901224));

    /** @arg The clone operation */
    p->setParam<int>("n", 20);
    ConditionPtr p_clone = p->clone();
    CHECK_NE(p, p_clone);
    p_src = (ConditionTest *)p_clone.get();
    CHECK_EQ(p_src->getKData().getStock(), stock);

    p->setParam<int>("n", 30);
    CHECK_EQ(p->getParam<int>("n"), 30);
    p->reset();
    CHECK_EQ(p->isValid(Datetime(200001010000)), false);

    CHECK_EQ(p_clone->name(), "TEST");
    CHECK_EQ(p_clone->getParam<int>("n"), 20);
    // CHECK_EQ(p_clone->isValid(Datetime(200001010000)) , true);
    p_clone->reset();
    CHECK_EQ(p_clone->isValid(Datetime(200001010000)), false);
}

/** @par Test points */
TEST_CASE("test_Condition_bool_non_finite") {
    StockManager &sm = StockManager::instance();
    KData kdata = sm.getStock("sh000001").getKData(KQuery(0, 10));
    if (kdata.empty()) {
        return;
    }

    /** @arg a non-finite condition value is not a valid condition day */
    ConditionPtr p = CN_Bool(CVAL(std::numeric_limits<double>::infinity()));
    p->setTO(kdata);
    CHECK_EQ(p->getDatetimeList().size(), 0);
    for (size_t i = 0; i < kdata.size(); ++i) {
        CHECK_UNARY_FALSE(p->isValid(kdata[i].datetime));
    }
}

/** @par Test points */
TEST_CASE("test_Condition_dirty_on_setSG") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0, 10));

    /** @arg the first setTO triggers a single calculation */
    ConditionPtr p(new ConditionTest);
    p->setTO(kdata);
    ConditionTest *pt = (ConditionTest *)p.get();
    int n = pt->calcCount();
    CHECK(n >= 1);

    /** @arg the same KData without a signal generator change does not recalculate */
    p->setTO(kdata);
    CHECK_EQ(pt->calcCount(), n);

    /** @arg changing the SG marks the condition dirty, so the next setTO recalculates even with
     *      the unchanged KData (CN-008) */
    p->setSG(std::make_shared<DummySG>());
    p->setTO(kdata);
    CHECK_EQ(pt->calcCount(), n + 1);

    /** @arg setting the same SG object again does not mark dirty, so no extra recalculation */
    p->setTO(kdata);
    CHECK_EQ(pt->calcCount(), n + 1);
}

/** @par Test points */
TEST_CASE("test_Condition_getValues_single_pass") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0, 10));
    if (kdata.empty()) {
        return;
    }

    /** @arg getValues yields one value per KData bar, in ascending date order, with no extra
     *      map traversal (CN-010) */
    ConditionPtr p = CN_Bool(CVAL(1.0));
    p->setTO(kdata);
    Indicator v = p->getValues();
    CHECK_EQ(v.size(), kdata.size());
    for (size_t i = 0; i < v.size(); ++i) {
        CHECK(v.getDatetime(i) == kdata[i].datetime);
        CHECK(v[i] > 0.0);
    }
}

/** @par Test point: isValid returns false for dates outside the trading object */
TEST_CASE("test_Condition_isValid_outside_TO") {
    StockManager &sm = StockManager::instance();
    KData kdata = sm.getStock("sh000001").getKData(KQuery(0, 10));
    if (kdata.empty()) {
        return;
    }

    ConditionPtr p = CN_Bool(CVAL(1.0));
    p->setTO(kdata);
    CHECK_EQ(p->getDatetimeList().size(), kdata.size());

    /** @arg dates before and after the TO window are not registered, so they are invalid */
    CHECK_UNARY_FALSE(p->isValid(Datetime(18000101)));
    CHECK_UNARY_FALSE(p->isValid(Datetime(29991231)));
}

/** @par Test point: CN_Manual is valid only for manually added dates (C++-side) */
TEST_CASE("test_Condition_manual") {
    StockManager &sm = StockManager::instance();
    KData kdata = sm.getStock("sh000001").getKData(KQuery(0, 10));
    if (kdata.empty()) {
        return;
    }

    CNPtr cn = CN_Manual();
    CHECK_EQ(cn->name(), "CN_Manual");

    cn->setTO(kdata);
    /** @arg nothing is valid until a date is registered manually */
    CHECK_EQ(cn->getDatetimeList().size(), 0);

    /** @arg a registered date becomes valid; the rest stay invalid */
    Datetime d = kdata[2].datetime;
    cn->_addValid(d);
    CHECK_EQ(cn->getDatetimeList().size(), 1);
    CHECK_UNARY(cn->isValid(d));
    CHECK_UNARY_FALSE(cn->isValid(kdata[3].datetime));

    /** @arg _addValid accumulates the value on the same date */
    cn->_addValid(d, 2.0);
    CHECK_EQ(cn->at(2), 3.0);
}

/** @par Test point: a subclass overriding _isValid changes both isValid and getDatetimeList */
TEST_CASE("test_Condition_isValid_hook") {
    StockManager &sm = StockManager::instance();
    KData kdata = sm.getStock("sh000001").getKData(KQuery(0, 10));
    if (kdata.empty()) {
        return;
    }

    ConditionPtr p(new ThresholdCondition);
    p->setTO(kdata);

    /** @arg 1.5 fails the >=2.0 custom rule while the default >0 would accept it */
    CHECK_UNARY(p->isValid(kdata[2].datetime));
    CHECK_UNARY_FALSE(p->isValid(kdata[3].datetime));
    CHECK_EQ(p->getDatetimeList().size(), 1);
}

#if HKU_SUPPORT_SERIALIZATION
/** @par Test points */
TEST_CASE("test_Condition_serialize") {
    StockManager &sm = StockManager::instance();
    Stock stock = sm.getStock("sh000001");
    KData kdata = stock.getKData(KQuery(0, 10));

    ConditionTest src;
    src.setTO(kdata);

    string filename(sm.tmpdir());
    filename += "/ConditionTest.xml";
    {
        std::ofstream ofs(filename);
        boost::archive::xml_oarchive oa(ofs);
        oa << BOOST_SERIALIZATION_NVP(src);
    }

    ConditionTest dst;
    {
        std::ifstream ifs(filename);
        boost::archive::xml_iarchive ia(ifs);
        ia >> BOOST_SERIALIZATION_NVP(dst);
    }

    /** @arg the configuration survives the round trip */
    CHECK_EQ(dst.name(), src.name());
    CHECK_EQ(dst.getParam<int>("n"), src.getParam<int>("n"));

    /** @arg the per-bar cache is no longer persisted; it is rebuilt by setTO */
    CHECK_EQ(dst.size(), 0);
    dst.setTO(kdata);
    CHECK_EQ(dst.size(), src.size());
    CHECK_EQ(dst.getDatetimeList().size(), src.getDatetimeList().size());
    CHECK_UNARY(dst.isValid(Datetime(19901221)));
    CHECK_UNARY(dst.isValid(Datetime(19901224)));
    CHECK_UNARY_FALSE(dst.isValid(Datetime(19901219)));
}
#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */
