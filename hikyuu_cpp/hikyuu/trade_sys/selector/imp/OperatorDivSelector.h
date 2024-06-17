/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#pragma once

#include "OperatorSelector.h"

namespace hku {

class HKU_API OperatorDivSelector : public OperatorSelector {
    OPERATOR_SELECTOR_IMP(OperatorDivSelector, "SE_Div")
    OPERATOR_SELECTOR_SERIALIZATION
};

}  // namespace hku