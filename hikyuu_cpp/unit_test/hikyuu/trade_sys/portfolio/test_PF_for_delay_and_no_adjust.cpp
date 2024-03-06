/*
 * test_SYS_Simple.cpp
 *
 *  Created on: 2013-4-20
 *      Author: fasiondog
 */

#include "doctest/doctest.h"
#include <hikyuu/StockManager.h>
#include <hikyuu/trade_manage/crt/crtTM.h>
#include <hikyuu/trade_sys/portfolio/crt/PF_Simple.h>
#include <hikyuu/trade_sys/selector/crt/SE_Fixed.h>
#include <hikyuu/trade_sys/allocatefunds/crt/AF_EqualWeight.h>

#include <hikyuu/trade_sys/system/crt/SYS_Simple.h>
#include <hikyuu/trade_sys/signal/crt/SG_CrossGold.h>
#include <hikyuu/trade_sys/moneymanager/crt/MM_FixedCount.h>
#include <hikyuu/indicator/crt/KDATA.h>
#include <hikyuu/indicator/crt/EMA.h>

using namespace hku;

/**
 * @defgroup test_Portfolio test_Portfolio
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

/** @par 检测点 全部为delay模式系统实例，不调整持仓*/
TEST_CASE("test_PF_for_delay_and_no_adjust") {
    StockManager& sm = StockManager::instance();

    SYSPtr sys = SYS_Simple();
    sys->setSG(SG_CrossGold(EMA(CLOSE(), 12), EMA(CLOSE(), 26)));
    sys->setMM(MM_FixedCount(100));
    SYSPtr pro_sys = sys->clone();

    TMPtr tm = crtTM(Datetime(199001010000L), 500000);
    SEPtr se = SE_Fixed();
    se->addStockList({sm["sz000001"], sm["sz000063"], sm["sz000651"]}, sys);
    AFPtr af = AF_EqualWeight();
    PFPtr pf = PF_Simple(tm, se, af);

    KQuery query = KQueryByDate(Datetime(201101010000L), Null<Datetime>(), KQuery::DAY);
    pf->run(query);

    /** @arg */
}

/** @} */
