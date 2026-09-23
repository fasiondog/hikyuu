/*
 * IVigor.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#pragma once
#ifndef IVIGOR_H_
#define IVIGOR_H_

#include "../Indicator.h"

namespace hku {

/*
 * Alexander Elder's force index
 * See "Come Into My Trading Room" (2007, Earthquake Press) (Alexander Elder) P131
 * Calculation formula: (today's close price - yesterday's close price) * today's volume
 * n: the period window used for the EMA smoothing, it must be an integer greater than 0
 */
class IVigor : public IndicatorImp {
    INDICATOR_IMP(IVigor)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IVigor();
    virtual ~IVigor() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* IVIGOR_H_ */
