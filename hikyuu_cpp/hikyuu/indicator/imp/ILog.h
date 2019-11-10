/*
 * ILog.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ILog_H_
#define INDICATOR_IMP_ILog_H_

#include "../Indicator.h"

namespace hku {

class ILog : public IndicatorImp {
    INDICATOR_IMP(ILog)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ILog();
    virtual ~ILog();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ILog_H_ */
