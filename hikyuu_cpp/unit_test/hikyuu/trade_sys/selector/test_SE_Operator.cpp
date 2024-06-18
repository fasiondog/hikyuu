/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cross.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/trade_sys/selector/crt/SE_Operator.h>

using namespace hku;

/**
 * @defgroup test_Selector_operator test_Selector_operator
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SE_AddValue") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg 数字 + 空指针 */
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

    /** @arg 空指针 + 数字 */
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

    /** @arg 选择器 + 数字 */
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

    /** @arg 数字 + 选择器 */
    se = 3.0 + se1;
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

/** @par 检测点 */
TEST_CASE("test_SE_SubValue") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg 数字 - 空指针 */
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

    /** @arg 空指针 - 数字 */
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

    /** @arg 选择器 - 数字 */
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

    /** @arg 数字 - 选择器 */
    se = 3.0 - se1;
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

/** @par 检测点 */
TEST_CASE("test_SE_MultiValue") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg 数字 * 空指针 */
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

    /** @arg 空指针 * 数字 */
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

    /** @arg 选择器 * 数字 */
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

    /** @arg 数字 * 选择器 */
    se = 3.0 * se1;
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

/** @par 检测点 */
TEST_CASE("test_SE_DivValue") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg 数字 / 空指针 */
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

    /** @arg 空指针 / 数字 */
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

    /** @arg 选择器 * 数字 */
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

    /** @arg 数字 / 选择器 */
    se = 3.0 / se1;
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

/** @par 检测点 */
TEST_CASE("test_SE_Add") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg 空指针 + 空指针 */
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

    /** @arg 选择器 + 空指针 */
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

    /** @arg 空指针 + 选择器 */
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

    /** @arg 正常两个选择器相加 */
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

    /** 异构并集, 必须使用 addSystem 加入实际的系统实例 */
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

    /** @arg 尝试 clone 操作 */
    CHECK_THROWS(se->clone());
}

/** @par 检测点 */
TEST_CASE("test_SE_Sub") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed();
    SEPtr se = se1 + se2;

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg 空指针 - 空指针 */
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

    /** @arg 选择器 - 空指针 */
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

    /** @arg 空指针 - 选择器 */
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

    /** @arg 正常两个选择器相- */
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

    /** @arg 尝试 clone 操作 */
    CHECK_THROWS(se->clone());
}

/** @par 检测点 */
TEST_CASE("test_SE_Multi") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed(0.5);
    SEPtr se = se1 + se2;

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg 空指针 * 空指针 */
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

    /** @arg 选择器 * 空指针 */
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

    /** @arg 空指针 * 选择器 */
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

    /** @arg 正常两个选择器相* */
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

    /** @arg 尝试 clone 操作 */
    CHECK_THROWS(se->clone());
}

/** @par 检测点 */
TEST_CASE("test_SE_Div") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    SGPtr sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
    MMPtr mm = MM_FixedCount(100);

    SEPtr se1 = SE_Fixed();
    SEPtr se2 = SE_Fixed(0.5);
    SEPtr se = se1 + se2;

    /** @arg 试图加入一个不存在的stock */
    CHECK_THROWS_AS(se->addStock(Stock(), sys), std::exception);

    /** @arg 试图加入一个空的系统策略原型 */
    CHECK_THROWS_AS(se->addStock(sm["sh600000"], SYSPtr()), std::exception);

    sys->setSG(sg);
    sys->setMM(mm);

    /** @arg 空指针 / 空指针 */
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

    /** @arg 选择器 / 空指针 */
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

    /** @arg 空指针 / 选择器 */
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

    /** @arg 正常两个选择器相除 */
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

    /** @arg 尝试 clone 操作 */
    CHECK_THROWS(se->clone());
}

/** @} */