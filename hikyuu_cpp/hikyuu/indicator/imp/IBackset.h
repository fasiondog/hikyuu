/*
 * IBackset.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-13
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IBACKSET_H_
#define INDICATOR_IMP_IBACKSET_H_

#include "../Indicator.h"

namespace hku {

class IBackset : public IndicatorImp {
    INDICATOR_IMP_SUPPORT_DYNAMIC_STEP(IBackset)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IBackset();
    virtual ~IBackset();

    virtual void _checkParam(const string& name) const override;
    virtual bool isSerial() const override {
        return true;
    }
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IBACKSET_H_ */
