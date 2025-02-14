/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

#pragma once

#include "../SignalBase.h"

namespace hku {

HKU_API SignalPtr operator+(const SignalPtr& sg1, const SignalPtr& sg2);
HKU_API SignalPtr operator-(const SignalPtr& sg1, const SignalPtr& sg2);
HKU_API SignalPtr operator*(const SignalPtr& sg1, const SignalPtr& sg2);
HKU_API SignalPtr operator/(const SignalPtr& sg1, const SignalPtr& sg2);

HKU_API SignalPtr operator+(const SignalPtr& sg, double value);
HKU_API SignalPtr operator-(const SignalPtr& sg, double value);
HKU_API SignalPtr operator*(const SignalPtr& sg, double value);
HKU_API SignalPtr operator/(const SignalPtr& sg, double value);

HKU_API SignalPtr operator+(double value, const SignalPtr& sg);
HKU_API SignalPtr operator-(double value, const SignalPtr& sg);
HKU_API SignalPtr operator*(double value, const SignalPtr& sg);
HKU_API SignalPtr operator/(double value, const SignalPtr& sg);

inline SignalPtr operator&(const SignalPtr& sg1, const SignalPtr& sg2) {
    return sg1 * sg2;
}

inline SignalPtr operator|(const SignalPtr& sg1, const SignalPtr& sg2) {
    return sg1 + sg2;
}

}  // namespace hku