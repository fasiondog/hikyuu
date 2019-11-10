/*
 * ICos.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ICOS_H_
#define INDICATOR_IMP_ICOS_H_

#include "../Indicator.h"

namespace hku {

class ICos : public IndicatorImp {
    INDICATOR_IMP(ICos)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICos();
    virtual ~ICos();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ICOS_H_ */
