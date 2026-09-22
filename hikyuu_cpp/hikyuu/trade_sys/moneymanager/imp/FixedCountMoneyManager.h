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
 * Fixed trade quantity money management strategy
 * @details A fixed quantity is bought every time; if the account balance is insufficient, enough
 * funds are deposited into the account to guarantee that the buy can be executed. That is, the
 * funds are always assumed to be sufficient.
 * @param n the quantity bought every time (it should be an integer multiple of the minimum trade
 *          quantity of the trading object, the program does not check this here)
 * @note 1) This strategy is mainly used to test and compare the results with the other strategies,
 * it does not conform to the reality itself. \n 2) This strategy does not judge the existing
 * positions; if a trade cannot be made with the existing positions, that judgment should be the
 * responsibility of the System itself
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
