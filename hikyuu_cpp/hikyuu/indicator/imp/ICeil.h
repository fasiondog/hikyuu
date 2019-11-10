/*
 * ICeil.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-15
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ICEIL_H_
#define INDICATOR_IMP_ICEIL_H_

#include "../Indicator.h"

namespace hku {

/**
 * 向上舍入 (向数值增大方向舍入)取整
 */
class ICeil : public IndicatorImp {
    INDICATOR_IMP(ICeil)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICeil();
    virtual ~ICeil();
};

} /* namespace hku */
#endif /* INDICATOR_IMP_ICEIL_H_ */
