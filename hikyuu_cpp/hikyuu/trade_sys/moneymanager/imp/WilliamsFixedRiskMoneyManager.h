/*
 * WilliamsFixedRiskMoneyManager.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_WILLIAMSFIXEDRISKMONEYMANAGER_H_
#define TRADE_SYS_MONEYMANAGER_IMP_WILLIAMSFIXEDRISKMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

class WilliamsFixedRiskMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(WilliamsFixedRiskMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    WilliamsFixedRiskMoneyManager();
    virtual ~WilliamsFixedRiskMoneyManager();
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_WILLIAMSFIXEDRISKMONEYMANAGER_H_ */
