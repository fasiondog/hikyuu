/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#pragma once

#include "../SelectorBase.h"

namespace hku {

HKU_API SelectorPtr operator+(const SelectorPtr& se1, const SelectorPtr& se2);
HKU_API SelectorPtr operator-(const SelectorPtr& se1, const SelectorPtr& se2);
HKU_API SelectorPtr operator*(const SelectorPtr& se1, const SelectorPtr& se2);
HKU_API SelectorPtr operator/(const SelectorPtr& se1, const SelectorPtr& se2);

inline SelectorPtr operator&(const SelectorPtr& se1, const SelectorPtr& se2) {
    return se1 * se2;
}

inline SelectorPtr operator|(const SelectorPtr& se1, const SelectorPtr& se2) {
    return se1 + se2;
}

HKU_API SelectorPtr operator+(const SelectorPtr& se, double value);
inline SelectorPtr operator+(double value, const SelectorPtr& se) {
    return se + value;
}

HKU_API SelectorPtr operator-(const SelectorPtr& se, double value);
HKU_API SelectorPtr operator-(double value, const SelectorPtr& se);

HKU_API SelectorPtr operator*(const SelectorPtr& se, double value);
inline SelectorPtr operator*(double value, const SelectorPtr& se) {
    return se * value;
}

HKU_API SelectorPtr operator/(const SelectorPtr& se, double value);
HKU_API SelectorPtr operator/(double value, const SelectorPtr& se);

}  // namespace hku