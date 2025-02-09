/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "OperatorValueSignal.h"

namespace hku {

class MulValueSignal : public OperatorValueSignal {
    OPERATOR_SIGNAL_IMP(MulValueSignal, "SG_MulValue")
    OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hku */
