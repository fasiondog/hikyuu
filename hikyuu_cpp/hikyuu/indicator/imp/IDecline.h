/*
 * IDecline.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-3
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IDECLINE_H_
#define INDICATOR_IMP_IDECLINE_H_

#include "../Indicator.h"

namespace hku {

class IDecline : public IndicatorImp {
    INDICATOR_IMP(IDecline)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDecline();
    virtual ~IDecline();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IDECLINE_H_ */
