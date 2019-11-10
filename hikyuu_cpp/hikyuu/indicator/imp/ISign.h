/*
 * ISign.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISIGN_H_
#define INDICATOR_IMP_ISIGN_H_

#include "../Indicator.h"

namespace hku {

class ISign : public IndicatorImp {
    INDICATOR_IMP(ISign)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ISign();
    virtual ~ISign();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ISIGN_H_ */
