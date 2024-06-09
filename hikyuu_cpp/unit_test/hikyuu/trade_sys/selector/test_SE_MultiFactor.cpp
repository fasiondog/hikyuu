/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-06-09
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_MultiFactor.h>
#include <hikyuu/trade_sys/selector/imp/MultiFactorSelector.h>
#include <hikyuu/trade_sys/signal/crt/SG_Cycle.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_Nothing.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/MA.h>
#include <hikyuu/indicator/crt/AMA.h>
#include <hikyuu/indicator/crt/EMA.h>

using namespace hku;

/**
 * @defgroup test_SE_MultiFactor test_SE_MultiFactor
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 */
TEST_CASE("test_SE_MultiFactor") {
    StockManager& sm = StockManager::instance();
    StockList stks{sm["sh600004"], sm["sh600005"], sm["sz000001"], sm["sz000002"]};
    Stock ref_stk = sm["sh000001"];
    KQuery query = KQuery(-100);
    IndicatorList src_inds{MA(CLOSE()), EMA(CLOSE())};

    auto sys = SYS_Simple(crtTM(), MM_Nothing());
    sys->setSG(SG_Cycle());
    sys->setParam<bool>("buy_delay", false);

    /** @arg 测试试图修改参数值为非法值 */
    auto ret = SE_MultiFactor(src_inds);
    CHECK_THROWS(ret->setParam<int>("ic_n", 0));
    CHECK_THROWS(ret->setParam<int>("ic_rolling_n", 0));
    CHECK_THROWS(ret->setParam<string>("mode", "MF"));

    /** @arg src_inds 为空，其余为默认参数 */
    CHECK_THROWS(SE_MultiFactor(IndicatorList{}));

    /** @arg 默认参数 */
    ret = SE_MultiFactor(src_inds, 10, 5, 120, ref_stk);
    ret->addStockList(stks, sys);
    auto proto_list = ret->getProtoSystemList();
    ret->calculate(proto_list, query);
}

//-----------------------------------------------------------------------------
// test export
//-----------------------------------------------------------------------------
#if HKU_SUPPORT_SERIALIZATION

/** @par 检测点 */
TEST_CASE("test_SE_MultiFactor_export") {}
#endif /* #if HKU_SUPPORT_SERIALIZATION */

/** @} */