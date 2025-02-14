/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "OperatorSignal.h"

namespace hku {

class DivSignal : public OperatorSignal {
    OPERATOR_SIGNAL_IMP(DivSignal, "SG_Div")
    OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION
};

} /* namespace hku */
