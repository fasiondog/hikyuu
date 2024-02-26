/*
 * IAd.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IAD_H_
#define INDICATOR_IMP_IAD_H_

#include "../Indicator.h"

namespace hku {

class IAd : public IndicatorImp {
    INDICATOR_IMP(IAd)
    INDICATOR_NEED_CONTEXT
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IAd();
    explicit IAd(const KData&);
    virtual ~IAd();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IAD_H_ */
