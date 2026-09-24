/*
 * IExist.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-19
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IEXIST_H_
#define INDICATOR_IMP_IEXIST_H_

#include "../Indicator.h"

namespace hku {

/*
 * Existence, EXIST(X,N) means the condition X exists within N periods
 */
class IExist : public IndicatorImp {
    INDICATOR_IMP(IExist)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IExist();
    virtual ~IExist() override;
    virtual void _checkParam(const string& name) const override;
    virtual bool supportIncrementCalculate() const override;
    virtual size_t min_increment_start() const override;
    virtual void _increment_calculate(const Indicator& ind, size_t start_pos) override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IEXIST_H_ */
