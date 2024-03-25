/*
 * FixedValueSlippage.h
 *
 *  Created on: 2016年5月7日
 *      Author: Administrator
 */

#pragma once
#ifndef TRADE_SYS_SLIPPAGE_IMP_FIXEDVALUESLIPPAGE_H_
#define TRADE_SYS_SLIPPAGE_IMP_FIXEDVALUESLIPPAGE_H_

#include "../SlippageBase.h"

namespace hku {

class FixedValueSlippage : public SlippageBase {
    SLIPPAGE_IMP(FixedValueSlippage)
    SLIPPAGE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedValueSlippage();
    virtual ~FixedValueSlippage();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */

#endif /* TRADE_SYS_SLIPPAGE_IMP_FIXEDVALUESLIPPAGE_H_ */
