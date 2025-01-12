/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-12
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/*
 * 替换指定数值，通常用于替换 Nan 值
 */
class IReplace : public IndicatorImp {
    INDICATOR_IMP(IReplace)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IReplace();
    virtual ~IReplace();
};

} /* namespace hku */
