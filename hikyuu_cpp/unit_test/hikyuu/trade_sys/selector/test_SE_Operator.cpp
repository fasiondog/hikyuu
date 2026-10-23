/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <fstream>
#include <set>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/trade_sys/selector/crt/SE_Logic.h>

using namespace hku;

/**
 * @defgroup test_Selector_operator test_Selector_operator
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par Test points */
TEST_CASE("test_SE_AddValue") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg Try to add a stock that does not exist */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg Try to add an empty system strategy prototype */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg A number + a null pointer */
    se = 3.0 + SEPtr();
    CHECK_EQ(se->name(), "SE_AddValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A null pointer + a number */
    se = SEPtr() + 3.0;
    CHECK_EQ(se->name(), "SE_AddValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A selector + a number */
    se = se1 + 2.0;
    CHECK_EQ(se->name(), "SE_AddValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 3.0);
    CHECK_EQ(result[1].weight, 3.0);
    CHECK_EQ(result[2].weight, 3.0);

    /** @arg A number + a selector (a fresh inner selector: the wrapper adopts the inner's
     *  existing prototypes, reusing se1 would double its list) */
    se = 3.0 + SE_Fixed();
    CHECK_EQ(se->name(), "SE_AddValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 4.0);
    CHECK_EQ(result[1].weight, 4.0);
    CHECK_EQ(result[2].weight, 4.0);
}

/** @par Test points */
TEST_CASE("test_SE_SubValue") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg Try to add a stock that does not exist */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg Try to add an empty system strategy prototype */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg A number - a null pointer */
    se = 3.0 - SEPtr();
    CHECK_EQ(se->name(), "SE_SubValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A null pointer - a number */
    se = SEPtr() - 3.0;
    CHECK_EQ(se->name(), "SE_SubValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A selector - a number */
    se = se1 - 2.0;
    CHECK_EQ(se->name(), "SE_SubValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, -1.0);
    CHECK_EQ(result[1].weight, -1.0);
    CHECK_EQ(result[2].weight, -1.0);

    /** @arg A number - a selector (a fresh inner selector, see the note in test_SE_AddValue) */
    se = 3.0 - SE_Fixed();
    CHECK_EQ(se->name(), "SE_SubValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 2.0);
    CHECK_EQ(result[1].weight, 2.0);
    CHECK_EQ(result[2].weight, 2.0);
}

/** @par Test points */
TEST_CASE("test_SE_MultiValue") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg Try to add a stock that does not exist */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg Try to add an empty system strategy prototype */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg A number * a null pointer */
    se = 3.0 * SEPtr();
    CHECK_EQ(se->name(), "SE_MultiValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A null pointer * a number */
    se = SEPtr() * 3.0;
    CHECK_EQ(se->name(), "SE_MultiValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A selector * a number */
    se = se1 * 2.0;
    CHECK_EQ(se->name(), "SE_MultiValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 2.0);
    CHECK_EQ(result[1].weight, 2.0);
    CHECK_EQ(result[2].weight, 2.0);

    /** @arg A number * a selector (a fresh inner selector, see the note in test_SE_AddValue) */
    se = 3.0 * SE_Fixed();
    CHECK_EQ(se->name(), "SE_MultiValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 3.0);
    CHECK_EQ(result[1].weight, 3.0);
    CHECK_EQ(result[2].weight, 3.0);
}

/** @par Test points */
TEST_CASE("test_SE_DivValue") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg Try to add a stock that does not exist */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg Try to add an empty system strategy prototype */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg A number / a null pointer */
    se = 3.0 / SEPtr();
    CHECK_EQ(se->name(), "SE_DivValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A null pointer / a number */
    se = SEPtr() / 3.0;
    CHECK_EQ(se->name(), "SE_DivValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A selector * a number */
    se = se1 / 2.0;
    CHECK_EQ(se->name(), "SE_DivValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 0.5);
    CHECK_EQ(result[1].weight, 0.5);
    CHECK_EQ(result[2].weight, 0.5);

    /** @arg A number / a selector (a fresh inner selector, see the note in test_SE_AddValue) */
    se = 3.0 / SE_Fixed();
    CHECK_EQ(se->name(), "SE_DivValue");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 3.0);
    CHECK_EQ(result[1].weight, 3.0);
    CHECK_EQ(result[2].weight, 3.0);
}

/** @par Test points */
TEST_CASE("test_SE_Add") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg Try to add a stock that does not exist */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg Try to add an empty system strategy prototype */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg A null pointer + a null pointer */
    se = SEPtr() + SEPtr();
    CHECK_EQ(se->name(), "SE_Add");
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A selector + a null pointer */
    se = se1 + SEPtr();
    CHECK_EQ(se->name(), "SE_Add");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 1.0);
    CHECK_EQ(result[1].weight, 1.0);
    CHECK_EQ(result[2].weight, 1.0);

    /** @arg A null pointer + a selector */
    se1->removeAll();
    se = SEPtr() + se1;
    CHECK_EQ(se->name(), "SE_Add");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 1.0);
    CHECK_EQ(result[1].weight, 1.0);
    CHECK_EQ(result[2].weight, 1.0);

    /** @arg The normal addition of two selectors */
    se1->removeAll();
    se2->removeAll();
    se = se1 + se2;
    CHECK_EQ(se->name(), "SE_Add");
    sys->setSG(sg);
    sys->setMM(mm);
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 2.0);
    CHECK_EQ(result[1].weight, 2.0);
    CHECK_EQ(result[2].weight, 2.0);

    /** A heterogeneous union: the real system instance must be added with addSystem */
    sys->reset();
    auto sys1 = sys->clone();
    sys1->setStock(sm["sh600000"]);
    auto sys2 = sys->clone();
    sys2->setStock(sm["sz000001"]);
    auto sys3 = sys->clone();
    sys3->setStock(sm["sz000002"]);

    se1 = SE_Fixed(0.2);
    se1->addSystem(sys1);
    se1->addSystem(sys2);
    CHECK_EQ(se1->getProtoSystemList().size(), 2);

    se2 = SE_Fixed(0.3);
    se2->addSystem(sys2);
    se2->addSystem(sys3);
    CHECK_EQ(se2->getProtoSystemList().size(), 2);

    se = se1 + se2;
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sz000001"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[1].sys->getStock());
    CHECK_EQ(sm["sh600000"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 0.5);
    CHECK_EQ(result[1].weight, 0.3);
    CHECK_EQ(result[2].weight, 0.2);

    /** @arg A combined selector is clonable and keeps its prototypes */
    SEPtr se_clone = se->clone();
    CHECK_EQ(se_clone->name(), "SE_Add");
    CHECK_EQ(se_clone->getProtoSystemList().size(), 3);
}

/** @par Test points */
TEST_CASE("test_SE_Sub") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg Try to add a stock that does not exist */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg Try to add an empty system strategy prototype */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg A null pointer - a null pointer */
    se = SEPtr() - SEPtr();
    CHECK_EQ(se->name(), "SE_Sub");
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A selector - a null pointer */
    se = se1 - SEPtr();
    CHECK_EQ(se->name(), "SE_Sub");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 1.0);
    CHECK_EQ(result[1].weight, 1.0);
    CHECK_EQ(result[2].weight, 1.0);

    /** @arg A null pointer - a selector */
    se1->removeAll();
    se = SEPtr() - se1;
    CHECK_EQ(se->name(), "SE_Sub");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, -1.0);
    CHECK_EQ(result[1].weight, -1.0);
    CHECK_EQ(result[2].weight, -1.0);

    /** @arg The normal subtraction of two selectors */
    se1->removeAll();
    se2->removeAll();
    se = se1 - se2;
    CHECK_EQ(se->name(), "SE_Sub");
    sys->setSG(sg);
    sys->setMM(mm);
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 0.0);
    CHECK_EQ(result[1].weight, 0.0);
    CHECK_EQ(result[2].weight, 0.0);

    /** @arg A combined selector is clonable and keeps its prototypes */
    SEPtr se_clone = se->clone();
    CHECK_EQ(se_clone->name(), "SE_Sub");
    CHECK_EQ(se_clone->getProtoSystemList().size(), 3);
}

/** @par Test points */
TEST_CASE("test_SE_Multi") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed(0.5);
    SEPtr se = se1 + se2;

    /** @arg Try to add a stock that does not exist */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg Try to add an empty system strategy prototype */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg A null pointer * a null pointer */
    se = SEPtr() * SEPtr();
    CHECK_EQ(se->name(), "SE_Multi");
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A selector * a null pointer */
    se1->removeAll();
    se = se1 * SEPtr();
    CHECK_EQ(se->name(), "SE_Multi");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A null pointer * a selector */
    se1->removeAll();
    se = SEPtr() * se1;
    CHECK_EQ(se->name(), "SE_Multi");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg The normal multiplication of two selectors */
    se1->removeAll();
    se2->removeAll();
    se = se1 * se2;
    CHECK_EQ(se->name(), "SE_Multi");
    sys->setSG(sg);
    sys->setMM(mm);
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 0.5);
    CHECK_EQ(result[1].weight, 0.5);
    CHECK_EQ(result[2].weight, 0.5);

    /** @arg A combined selector is clonable and keeps its prototypes */
    SEPtr se_clone = se->clone();
    CHECK_EQ(se_clone->name(), "SE_Multi");
    CHECK_EQ(se_clone->getProtoSystemList().size(), 3);
}

/** @par Test points */
TEST_CASE("test_SE_Div") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed(0.5);
    SEPtr se = se1 + se2;

    /** @arg Try to add a stock that does not exist */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg Try to add an empty system strategy prototype */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg A null pointer / a null pointer */
    se = SEPtr() / SEPtr();
    CHECK_EQ(se->name(), "SE_Div");
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A selector / a null pointer */
    se1->removeAll();
    se = se1 / SEPtr();
    CHECK_EQ(se->name(), "SE_Div");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg A null pointer / a selector */
    se1->removeAll();
    se = SEPtr() / se1;
    CHECK_EQ(se->name(), "SE_Div");
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    /** @arg The normal division of two selectors */
    se1->removeAll();
    se2->removeAll();
    se = se1 / se2;
    CHECK_EQ(se->name(), "SE_Div");
    sys->setSG(sg);
    sys->setMM(mm);
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 2.0);
    CHECK_EQ(result[1].weight, 2.0);
    CHECK_EQ(result[2].weight, 2.0);

    /** @arg A combined selector is clonable and keeps its prototypes */
    SEPtr se_clone = se->clone();
    CHECK_EQ(se_clone->name(), "SE_Div");
    CHECK_EQ(se_clone->getProtoSystemList().size(), 3);
}

/** @par Test points */
TEST_CASE("test_SE_Operator_clone_combined") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    sys->setMM(MM_FixedCount(100));
    sys->reset();

    auto p1 = sys->clone();
    p1->setStock(sm["sh600000"]);
    auto p2 = sys->clone();
    p2->setStock(sm["sz000001"]);
    auto p3 = sys->clone();
    p3->setStock(sm["sz000002"]);

    /** @arg The same prototype instance makes p2 the intersection of se1 and se2 */
    SEPtr se1 = SE_Fixed(0.25);
    se1->addSystem(p1);
    se1->addSystem(p2);
    SEPtr se2 = SE_Fixed(0.5);
    se2->addSystem(p2);
    se2->addSystem(p3);

    SEPtr se = se1 + se2;
    CHECK_EQ(se->getProtoSystemList().size(), 3);

    /** @arg A combined selector is cloned instead of throwing */
    SEPtr clone = se->clone();
    CHECK_EQ(clone->name(), "SE_Add");
    CHECK_EQ(clone->getProtoSystemList().size(), 3);

    /** @arg The cloned prototype list contains no duplicate instances */
    std::set<const System*> uniq;
    for (const auto& s : clone->getProtoSystemList()) {
        CHECK_UNARY(uniq.insert(s.get()).second);
    }

    /** @arg The intersection survives the reset that rebuilds the two operand sets */
    clone->reset();
    for (const auto& s : clone->getProtoSystemList()) {
        clone->bindRealToProto(s, s);
    }
    clone->calculate(clone->getProtoSystemList(), KQuery(-20));
    auto result = clone->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    CHECK_EQ(sm["sz000001"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[1].sys->getStock());
    CHECK_EQ(sm["sh600000"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 0.75);
    CHECK_EQ(result[1].weight, 0.5);
    CHECK_EQ(result[2].weight, 0.25);

    /** @arg The intersection operator shares the single prototype instance after clone */
    SEPtr its = (se1 & se2)->clone();
    its->reset();
    for (const auto& s : its->getProtoSystemList()) {
        its->bindRealToProto(s, s);
    }
    its->calculate(its->getProtoSystemList(), KQuery(-20));
    auto its_result = its->getSelected(Datetime(200001010000L));
    CHECK_EQ(its_result.size(), 1);
    CHECK_EQ(sm["sz000001"], its_result[0].sys->getStock());
    CHECK_EQ(its_result[0].weight, 0.125);
}

/** @par Test points */
TEST_CASE("test_SE_ValueSelector_clone") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    sys->setMM(MM_FixedCount(100));

    // The wrapped inner selector is a logical operator holding the prototypes, so the clone must
    // expose the very prototype instance set of the inner selector (a flat SE_Fixed wrapper would
    // hide the distinction since it never consults its own prototype list)
    SEPtr se1 = SE_Fixed();
    se1->addStockList({sm["sh600000"]}, sys);
    SEPtr se2 = SE_Fixed();
    se2->addStockList({sm["sz000001"]}, sys);
    SEPtr se = (se1 + se2) + 2.0;
    CHECK_EQ(se->name(), "SE_AddValue");
    auto proto_sys_list = se->getProtoSystemList();
    REQUIRE_EQ(proto_sys_list.size(), 2);

    /** @arg The combined selector is cloned with the same prototype layout */
    SEPtr clone = se->clone();
    CHECK_EQ(clone->name(), "SE_AddValue");
    const auto& clone_proto_list = clone->getProtoSystemList();
    REQUIRE_EQ(clone_proto_list.size(), 2);
    for (size_t i = 0; i < clone_proto_list.size(); i++) {
        CHECK_EQ(clone_proto_list[i]->getStock(), proto_sys_list[i]->getStock());
        /** @arg The clone holds its own prototype instances */
        CHECK_UNARY(clone_proto_list[i] != proto_sys_list[i]);
    }

    /** @arg The binding forwarding reaches the inner logical selector and the clone still selects
     *  the same targets with the value offset applied (before the fix the outer clone carried a
     *  second prototype instance set disjoint from the inner selector and selected nothing) */
    for (const auto& s : clone_proto_list) {
        clone->bindRealToProto(s, s);
    }
    clone->calculate(clone_proto_list, KQuery(-20));
    auto result = clone->getSelected(Datetime(200001010000L));
    REQUIRE_EQ(result.size(), 2);
    bool has_stk1 = false;
    bool has_stk2 = false;
    for (const auto& sw : result) {
        CHECK_EQ(sw.weight, 3.0);
        has_stk1 |= (sw.sys->getStock() == sm["sh600000"]);
        has_stk2 |= (sw.sys->getStock() == sm["sz000001"]);
    }
    CHECK_UNARY(has_stk1);
    CHECK_UNARY(has_stk2);
}

/** @par Test points */
TEST_CASE("test_SE_Nested_logic_bind") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    sys->setMM(MM_FixedCount(100));

    // (a + b) + c: the middle operand is itself a logical operator, the real-to-proto binding
    // must reach it, otherwise the nested level would silently select nothing
    SEPtr se1 = SE_Fixed();
    se1->addStockList({sm["sh600000"]}, sys);
    SEPtr se2 = SE_Fixed();
    se2->addStockList({sm["sz000001"]}, sys);
    SEPtr se3 = SE_Fixed();
    se3->addStockList({sm["sz000002"]}, sys);
    SEPtr se = (se1 + se2) + se3;
    CHECK_EQ(se->name(), "SE_Add");
    auto proto_sys_list = se->getProtoSystemList();
    REQUIRE_EQ(proto_sys_list.size(), 3);
    for (const auto& s : proto_sys_list) {
        se->bindRealToProto(s, s);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    REQUIRE_EQ(result.size(), 3);
    bool has_stk1 = false;
    bool has_stk2 = false;
    bool has_stk3 = false;
    for (const auto& sw : result) {
        CHECK_EQ(sw.weight, 1.0);
        has_stk1 |= (sw.sys->getStock() == sm["sh600000"]);
        has_stk2 |= (sw.sys->getStock() == sm["sz000001"]);
        has_stk3 |= (sw.sys->getStock() == sm["sz000002"]);
    }
    CHECK_UNARY(has_stk1);
    CHECK_UNARY(has_stk2);
    CHECK_UNARY(has_stk3);
}

/** @par Test points */
TEST_CASE("test_SE_DivValue_byZero") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    sys->setMM(MM_FixedCount(100));

    /** @arg A zero divisor yields an undefined weight, reported as Null instead of inf / NaN */
    SEPtr se = SE_Fixed() / 0.0;
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_EQ(result.size(), 3);
    for (const auto& sw : result) {
        CHECK_UNARY(std::isnan(sw.weight));
    }

    /** @arg A non-zero divisor keeps the normal quotient */
    SEPtr se2 = SE_Fixed() / 2.0;
    se2->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    auto proto_sys_list2 = se2->getProtoSystemList();
    se2->calculate(proto_sys_list2, KQuery(-20));
    auto result2 = se2->getSelected(Datetime(200001010000L));
    CHECK_EQ(result2.size(), 3);
    CHECK_EQ(result2[0].weight, 0.5);

    /** @arg A number divided by a selector with a zero value yields Null weights */
    SEPtr se3 = 2.0 / SE_Fixed(0.0);
    se3->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    auto proto_sys_list3 = se3->getProtoSystemList();
    se3->calculate(proto_sys_list3, KQuery(-20));
    auto result3 = se3->getSelected(Datetime(200001010000L));
    CHECK_EQ(result3.size(), 3);
    for (const auto& sw : result3) {
        CHECK_UNARY(std::isnan(sw.weight));
    }

    /** @arg A selector divided by another selector with a zero value yields Null weights */
    SEPtr se4 = SE_Fixed(1.0) / SE_Fixed(0.0);
    se4->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    auto proto_sys_list4 = se4->getProtoSystemList();
    // The se-se operator routes the real systems by the real-to-proto binding
    for (const auto& s : proto_sys_list4) {
        se4->bindRealToProto(s, s);
    }
    se4->calculate(proto_sys_list4, KQuery(-20));
    auto result4 = se4->getSelected(Datetime(200001010000L));
    CHECK_EQ(result4.size(), 3);
    for (const auto& sw : result4) {
        CHECK_UNARY(std::isnan(sw.weight));
    }
}

#if HKU_SUPPORT_SERIALIZATION

/** @par Test points */
TEST_CASE("test_SE_Operator_export") {
    StockManager& sm = StockManager::instance();

    TMPtr tm = crtTM(Datetime(20010101), 100000);
    SYSPtr sys = SYS_Simple();
    sys->setTM(tm);
    sys->setSG(SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
    sys->setMM(MM_FixedCount(100));
    sys->reset();

    auto p1 = sys->clone();
    p1->setStock(sm["sh600000"]);
    auto p2 = sys->clone();
    p2->setStock(sm["sz000001"]);
    auto p3 = sys->clone();
    p3->setStock(sm["sz000002"]);

    // se1: {p1, p2}, se2: {p2, p3}, p2 is the shared intersection prototype
    SEPtr se1 = SE_Fixed(0.25);
    se1->addSystem(p1);
    se1->addSystem(p2);
    SEPtr se2 = SE_Fixed(0.5);
    se2->addSystem(p2);
    se2->addSystem(p3);

    auto round_trip = [&sm](const SEPtr& se, const string& tag) {
        string filename(sm.tmpdir());
        filename += fmt::format("/SE_Operator_{}.xml", tag);
        {
            std::ofstream ofs(filename);
            boost::archive::xml_oarchive oa(ofs);
            oa << BOOST_SERIALIZATION_NVP(se);
        }

        SEPtr loaded;
        {
            std::ifstream ifs(filename);
            boost::archive::xml_iarchive ia(ifs);
            ia >> BOOST_SERIALIZATION_NVP(loaded);
        }
        return loaded;
    };

    auto check_protos = [](const SEPtr& se) {
        /** @arg The prototype list is rebuilt, not doubled, after the round-trip */
        const auto& protos = se->getProtoSystemList();
        REQUIRE_EQ(protos.size(), 3);
        std::set<string> codes;
        for (const auto& s : protos) {
            CHECK_UNARY(codes.insert(s->getStock().market_code()).second);
        }
        return protos;
    };

    /** @arg SE_Add round-trip keeps the union prototypes and the union weights */
    SEPtr se = round_trip(se1 + se2, "SE_Add");
    CHECK_EQ(se->name(), "SE_Add");
    auto protos = check_protos(se);
    for (const auto& s : protos) {
        se->bindRealToProto(s, s);
    }
    se->calculate(protos, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    REQUIRE_EQ(result.size(), 3);
    CHECK_EQ(sm["sz000001"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[1].sys->getStock());
    CHECK_EQ(sm["sh600000"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 0.75);
    CHECK_EQ(result[1].weight, 0.5);
    CHECK_EQ(result[2].weight, 0.25);

    /** @arg SE_Sub round-trip keeps the union prototypes and the difference weights */
    se = round_trip(se1 - se2, "SE_Sub");
    CHECK_EQ(se->name(), "SE_Sub");
    protos = check_protos(se);
    for (const auto& s : protos) {
        se->bindRealToProto(s, s);
    }
    se->calculate(protos, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    REQUIRE_EQ(result.size(), 3);
    CHECK_EQ(sm["sh600000"], result[0].sys->getStock());
    CHECK_EQ(sm["sz000001"], result[1].sys->getStock());
    CHECK_EQ(sm["sz000002"], result[2].sys->getStock());
    CHECK_EQ(result[0].weight, 0.25);
    CHECK_EQ(result[1].weight, -0.25);
    CHECK_EQ(result[2].weight, -0.5);

    /** @arg SE_Multi round-trip keeps the intersection prototype and the product weight */
    se = round_trip(se1 * se2, "SE_Multi");
    CHECK_EQ(se->name(), "SE_Multi");
    protos = check_protos(se);
    for (const auto& s : protos) {
        se->bindRealToProto(s, s);
    }
    se->calculate(protos, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    REQUIRE_EQ(result.size(), 1);
    CHECK_EQ(sm["sz000001"], result[0].sys->getStock());
    CHECK_EQ(result[0].weight, 0.125);

    /** @arg SE_Div round-trip keeps the intersection prototype and the quotient weight */
    se = round_trip(se1 / se2, "SE_Div");
    CHECK_EQ(se->name(), "SE_Div");
    protos = check_protos(se);
    for (const auto& s : protos) {
        se->bindRealToProto(s, s);
    }
    se->calculate(protos, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    REQUIRE_EQ(result.size(), 1);
    CHECK_EQ(sm["sz000001"], result[0].sys->getStock());
    CHECK_EQ(result[0].weight, 0.5);
}

#endif /* HKU_SUPPORT_SERIALIZATION */

/** @} */