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

    se = 3.0 + SEPtr();
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    se = se1 + SEPtr();
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

    se = se1 + 2.0;
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

    se = 3.0 + se1;
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

    se = SEPtr() + SEPtr();
    se->addStock(sm["sh600000"], sys);
    se->addStock(sm["sz000001"], sys);
    se->addStock(sm["sz000002"], sys);

    auto proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    se->calculate(proto_sys_list, KQuery(-20));
    auto result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    se = se1 + se2;
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

    se = 3.0 + SEPtr();
    se->addStockList({sm["sh600000"], sm["sz000001"], sm["sz000002"]}, sys);
    proto_sys_list = se->getProtoSystemList();
    CHECK_EQ(proto_sys_list.size(), 3);
    for (const auto& sys : proto_sys_list) {
        se->bindRealToProto(sys, sys);
    }
    se->calculate(proto_sys_list, KQuery(-20));
    result = se->getSelected(Datetime(200001010000L));
    CHECK_UNARY(result.empty());

    se = se1 + SEPtr();
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

    se = se1 + 2.0;
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

    se = 3.0 + se1;
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

/** @} */