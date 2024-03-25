/*
 * FixedCapitalMoneyManager.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_FIXEDCAPITALMONEYMANAGER_H_
#define TRADE_SYS_MONEYMANAGER_IMP_FIXEDCAPITALMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

class FixedCapitalMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(FixedCapitalMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedCapitalMoneyManager();
    virtual ~FixedCapitalMoneyManager();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_FIXEDCAPITALMONEYMANAGER_H_ */
