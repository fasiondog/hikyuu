/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "OperatorValueSignal.h"

namespace hku {

class DivValueSignal : public OperatorValueSignal {
    OPERATOR_SIGNAL_IMP(DivValueSignal, "SG_DivValue")
    OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    DivValueSignal(double value, const SignalPtr& sg);
};

} /* namespace hku */
