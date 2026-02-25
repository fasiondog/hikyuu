/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IWMA_H_
#define INDICATOR_IMP_IWMA_H_

#include "../Indicator.h"

namespace hku {

/*
 * WMA(X,N):X的N日加权移动平均.算法:Yn=(1*X1+2*X2+...+n*Xn)/(1+2+...+n)。
 */
class IWma : public IndicatorImp {
    INDICATOR_IMP(IWma)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IWma();
    virtual ~IWma() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IWMA_H_ */
