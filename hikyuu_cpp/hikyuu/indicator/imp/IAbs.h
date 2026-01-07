/*
 * IAbs.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IABS_H_
#define INDICATOR_IMP_IABS_H_

#include "../Indicator.h"

namespace hku {

class IAbs : public IndicatorImp {
    INDICATOR_IMP(IAbs)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAbs();
    virtual ~IAbs();

    virtual bool supportIncrementCalculate() const override {
        return true;
    }

    virtual void _increment_one_cycle(const Indicator& ind, size_t pos, size_t r) override {
        _set(std::abs(ind.get(pos, r)), pos, r);
    }
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IABS_H_ */
