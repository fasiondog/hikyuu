/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-19
 *      Author: fasiondog
 */

#pragma once
#ifndef TRADE_SYS_MONEYMANAGER_IMP_FIXEDCAPITAL_FUNDS_MM_H_
#define TRADE_SYS_MONEYMANAGER_IMP_FIXEDCAPITAL_FUNDS_MM_H_

#include "../MoneyManagerBase.h"

namespace hku {

class FixedCapitalFundsMM : public MoneyManagerBase {
    MONEY_MANAGER_IMP(FixedCapitalFundsMM)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedCapitalFundsMM();
    virtual ~FixedCapitalFundsMM();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_MONEYMANAGER_IMP_FIXEDCAPITAL_FUNDS_MM_H_ */
