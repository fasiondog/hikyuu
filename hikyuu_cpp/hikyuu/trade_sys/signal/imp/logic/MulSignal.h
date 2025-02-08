/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "OperatorSignal.h"

namespace hku {

class MulSignal : public OperatorSignal {
    OPERATOR_SIGNAL_IMP(MulSignal, "SG_Mul")
    OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hku */
