/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#include "../../test_config.h"
#include "hikyuu/trade_sys/signal/crt/SG_Logic.h"
#include "hikyuu/trade_sys/signal/crt/SG_Manual.h"

using namespace hku;

/**
 * @defgroup test_Signal test_Signal_logic
 * @ingroup test_hikyuu_trade_sys_suite
 * @{
 */

static void reset_expect(std::map<Datetime, double>& expect) {
    for (auto& iter : expect) {
        iter.second = 0.0;
    }
}

/** @par 检测点 */
TEST_CASE("test_SG_Add") {
    auto k = getKData("sz000001", KQueryByDate(Datetime(20111108), Datetime(20111201)));
    std::map<Datetime, double> expect;
    for (size_t i = 0; i < k.size(); i++) {
        HKU_INFO("{}", k[i]);
        expect[k[i].datetime] = 0.0;
    }

    /** @arg sg1, sg2 不存在重叠 */
    auto sg1 = SG_Manual();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));

    auto sg2 = SG_Manual();
    sg2->_addBuySignal(Datetime(20111115));
    sg2->_addSellSignal(Datetime(20111116));

    auto ret = sg1 + sg2;
    ret->setParam<bool>("alternate", true);
    ret->setTO(k);

    expect[Datetime(20111110)] = 1.0;
    expect[Datetime(20111114)] = -1.0;
    expect[Datetime(20111115)] = 1.0;
    expect[Datetime(20111116)] = -1.0;
    for (auto iter = expect.begin(); iter != expect.end(); ++iter) {
        CHECK_EQ(ret->getValue(iter->first), iter->second);
    }

    /** @arg sg1, sg2 存在重叠 */
    sg1->reset();
    sg2->reset();
    sg1->_addBuySignal(Datetime(20111110));
    sg1->_addSellSignal(Datetime(20111114));
    sg2->_addBuySignal(Datetime(20111115));
    sg2->_addSellSignal(Datetime(20111116));

    reset_expect(expect);
}

/** @} */