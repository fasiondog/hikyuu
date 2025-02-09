/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "OperatorValueSignal.h"

namespace hku {

class SubValueSignal : public OperatorValueSignal {
    OPERATOR_SIGNAL_IMP(SubValueSignal, "SG_AddValue")
    OPERATOR_SIGNAL_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    SubValueSignal(double value, const SignalPtr& sg);
};

} /* namespace hku */
