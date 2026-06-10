/*
 * ISkewness.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISKEWNESS_H_
#define INDICATOR_IMP_ISKEWNESS_H_

#include "../Indicator.h"

namespace hku {

/*
 * 计算未调整的总体偏度
 */
class ISkewness : public hku::IndicatorImp {
    INDICATOR_IMP(ISkewness)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISkewness();
    virtual ~ISkewness() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ISKEWNESS_H_ */