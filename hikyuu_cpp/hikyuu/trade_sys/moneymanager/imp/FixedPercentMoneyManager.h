/*
 * PercentRiskMoneyManager.h
 *
 *  Created on: 2015-4-4
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_FIXEDPERCENTMONEYMANAGER_H_
#define TRADE_SYS_MONEYMANAGER_IMP_FIXEDPERCENTMONEYMANAGER_H_

#include "../MoneyManagerBase.h"

namespace hku {

/*
 * Percentage risk model
 * See "Financial Freedom Through Electronic Day Trading" (June 2008, China Machine Press) by Van
 * K. Tharp, P312
 * Formula: P (position size) = C (total risk) / R (risk per share) [here C, the cash, is the total
 * risk]
 * Parameter: percent: the percentage of the total risk of every trade in the total assets, e.g.
 * 0.02 means 2% of the total assets
 */
class HKU_API FixedPercentMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(FixedPercentMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedPercentMoneyManager();
    virtual ~FixedPercentMoneyManager();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_FIXEDPERCENTMONEYMANAGER_H_ */
