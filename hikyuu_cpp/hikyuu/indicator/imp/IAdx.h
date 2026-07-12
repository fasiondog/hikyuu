#pragma once
#ifndef INDICATOR_IMP_IAdx_H_
#define INDICATOR_IMP_IAdx_H_

#include "../Indicator.h"

namespace hku {

// ADX 平均趋向指数实现类
// 采用威尔德（Wilder）原始公式，周期N=14
// 平滑系数 = 1/N，初始值为N周期简单平均
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