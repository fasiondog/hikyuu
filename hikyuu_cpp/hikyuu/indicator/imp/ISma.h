/*
 * ISma.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISMA_H_
#define INDICATOR_IMP_ISMA_H_

#include "../Indicator.h"

namespace hku {

class ISma : public IndicatorImp {
    INDICATOR_IMP(ISma)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISma();
    virtual ~ISma();

    virtual void _checkParam(const string& name) const override;
    virtual void _dyn_calculate(const Indicator&) override;

private:
    void _dyn_one_circle(const Indicator& ind, size_t curPos, int n, double m);
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ISMA_H_ */
