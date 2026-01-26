/*
 * IVarp.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IVARP_H_
#define INDICATOR_IMP_IVARP_H_

#include "../Indicator.h"

namespace hku {

/*
 * 估算总体样本方差
 */
class IVarp : public hku::IndicatorImp {
    INDICATOR_IMP(IVarp)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IVarp();
    virtual ~IVarp();
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IVARP_H_ */
