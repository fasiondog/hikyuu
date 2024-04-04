/*
 * FixedRiskMM.h
 *
 *  Created on: 2016年5月1日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_FIXEDRISKMONEYMANAGER_H_
#define TRADE_SYS_MONEYMANAGER_IMP_FIXEDRISKMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

class FixedRiskMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(FixedRiskMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedRiskMoneyManager();
    virtual ~FixedRiskMoneyManager();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_FIXEDRISKMONEYMANAGER_H_ */
