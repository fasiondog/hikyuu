/*
 * NotMoneyManager.h
 *
 *  Created on: 2017-5-22
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_NOTMONEYMANAGER_H_
#define TRADE_SYS_MONEYMANAGER_IMP_NOTMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

/*
 * No money management strategy, i.e. it buys as much as the available money allows and opens a
 * short as much as the net assets allow
 */
class NotMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(NotMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NotMoneyManager();
    virtual ~NotMoneyManager();

protected:
    virtual double _getSellShortNumber(const Datetime& datetime, const Stock& stock, price_t price,
                                       price_t risk, SystemPart from) override;
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_NOTMONEYMANAGER_H_ */
