/*
 * ITsRank.h
 *
 *  Created on: 2026年6月9日
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_HKU_ITSRANK_H_
#define INDICATOR_IMP_HKU_ITSRANK_H_

#include "../Indicator.h"

namespace hku {

class ITsRank : public IndicatorImp {
    INDICATOR_IMP(ITsRank)
    INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITsRank();
    virtual ~ITsRank() override;
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_HKU_ITSRANK_H_ */