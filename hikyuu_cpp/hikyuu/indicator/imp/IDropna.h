/*
 * IDropna.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IDROPNA_H_
#define INDICATOR_IMP_IDROPNA_H_

#include "../Indicator.h"

namespace hku {

class IDropna : public IndicatorImp {
    INDICATOR_IMP(IDropna)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IDropna();
    virtual ~IDropna();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_IDROPNA_H_ */
