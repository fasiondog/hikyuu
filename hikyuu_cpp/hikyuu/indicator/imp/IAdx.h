#pragma once
#ifndef INDICATOR_IMP_IAdx_H_
#define INDICATOR_IMP_IAdx_H_

#include "../Indicator.h"

namespace hku {

// Implementation class of the ADX average directional index
// The original formula of Wilder is used, with the period N = 14
// Smoothing coefficient = 1/N, the initial value is the simple average of N periods
class IAdx : public IndicatorImp {
    INDICATOR_IMP(IAdx)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAdx();
    virtual ~IAdx() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IAdx_H_ */