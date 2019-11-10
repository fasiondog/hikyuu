/*
 * NotMoneyManager.h
 *
 *  Created on: 2017年5月22日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_NOTMONEYMANAGER_H_
#define TRADE_SYS_MONEYMANAGER_IMP_NOTMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

/*
 * 无资金管理策略，即有多少钱买多少
 */
class NotMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(NotMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NotMoneyManager();
    virtual ~NotMoneyManager();
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_NOTMONEYMANAGER_H_ */
