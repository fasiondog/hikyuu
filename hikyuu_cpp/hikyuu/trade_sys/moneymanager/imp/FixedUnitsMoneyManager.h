/*
 * FixedUnitsMoneyManager.h
 *
 *  Created on: 2016年5月3日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_FIXEDUNITSMONEYMANAGER_H_
#define TRADE_SYS_MONEYMANAGER_IMP_FIXEDUNITSMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

class FixedUnitsMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(FixedUnitsMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedUnitsMoneyManager();
    virtual ~FixedUnitsMoneyManager();
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_FIXEDUNITSMONEYMANAGER_H_ */
