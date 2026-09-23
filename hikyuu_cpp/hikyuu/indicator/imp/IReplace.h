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
 * Replace the given value, it is usually used to replace the Nan values
 */
class IReplace : public IndicatorImp {
    INDICATOR_IMP(IReplace)
    INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    IReplace();
    virtual ~IReplace() override;
};

} /* namespace hku */
