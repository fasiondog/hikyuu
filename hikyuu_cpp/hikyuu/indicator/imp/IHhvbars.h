/*
 * IHhvbars.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IHHVBARS_H_
#define INDICATOR_IMP_IHHVBARS_H_

#include "../Indicator.h"

namespace hku {

class IHhvbars : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IHhvbars)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IHhvbars();
    virtual ~IHhvbars();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IHHVBARS_H_ */
