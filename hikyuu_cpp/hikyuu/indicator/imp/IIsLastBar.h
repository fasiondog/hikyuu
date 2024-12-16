/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-16
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IISLASTBAR_H_
#define INDICATOR_IMP_IISLASTBAR_H_

#include "../Indicator.h"

namespace hku {

class IIsLastBar : public IndicatorImp {
    INDICATOR_IMP(IIsLastBar)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IIsLastBar();
    explicit IIsLastBar(const KData&);
    virtual ~IIsLastBar();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IISLASTBAR_H_ */
