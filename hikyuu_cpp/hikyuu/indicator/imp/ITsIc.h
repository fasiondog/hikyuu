/*
 * ITsIc.h
 *
 *  Created on: 2026-11-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_HKU_ITSIC_H_
#define INDICATOR_IMP_HKU_ITSIC_H_

#include "../Indicator.h"

namespace hku {

class ITsIc : public IndicatorImp {
    INDICATOR_IMP(ITsIc)
    INDICATOR_IMP_SUPPORT_INCREMENT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ITsIc();
    virtual ~ITsIc() override;
    virtual void _checkParam(const string& name) const override;
    virtual size_t min_increment_start() const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_HKU_ITSIC_H_ */
