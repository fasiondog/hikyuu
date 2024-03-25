/*
 * FixedCountMoneyManager.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef FIXEDCOUNTMONEYMANAGER_H_
#define FIXEDCOUNTMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

/**
 * 固定交易数量资金管理策略
 * @details 每次买入固定的数量，如果帐户余额不足，则向帐户中存入足够的资金，保证能够执行买入。
 *          即，假设资金总是充足的。
 * @param n 每次买入的数量（应该是交易对象最小交易数量的整数，此处程序没有此进行判断）
 * @note 1) 该策略主要用于测试和其他策略进行比较结果，本身不符合现实。\n
 *       2) 该策略并不判断已有的持仓情况，如果在已有持仓情况下不能交易，则该判断应为System本身的责任
 */
class FixedCountMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(FixedCountMoneyManager)

    virtual double _getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                       price_t risk, SystemPart from) override;
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedCountMoneyManager();
    virtual ~FixedCountMoneyManager();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* FIXEDCOUNTMONEYMANAGER_H_ */
