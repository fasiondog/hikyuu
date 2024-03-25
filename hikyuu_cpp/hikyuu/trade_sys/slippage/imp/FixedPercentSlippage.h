/*
 * FixedPercentSlippage.h
 *
 *  Created on: 2016年5月7日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_SLIPPAGE_IMP_FIXEDPERCENTSLIPPAGE_H_
#define TRADE_SYS_SLIPPAGE_IMP_FIXEDPERCENTSLIPPAGE_H_

#include "../SlippageBase.h"

namespace hku {

class FixedPercentSlippage : public SlippageBase {
    SLIPPAGE_IMP(FixedPercentSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedPercentSlippage();
    virtual ~FixedPercentSlippage();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_SLIPPAGE_IMP_FIXEDPERCENTSLIPPAGE_H_ */
