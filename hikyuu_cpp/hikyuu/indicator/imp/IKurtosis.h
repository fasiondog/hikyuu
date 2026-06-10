/*
 * IKurtosis.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IKURTOSIS_H_
#define INDICATOR_IMP_IKURTOSIS_H_

#include "../Indicator.h"

namespace hku {

/*
 * 计算未调整的总体峰度（超额峰度 = 峰度 - 3）
 */
class IKurtosis : public hku::IndicatorImp {
    INDICATOR_IMP(IKurtosis)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IKurtosis();
    virtual ~IKurtosis() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IKURTOSIS_H_ */