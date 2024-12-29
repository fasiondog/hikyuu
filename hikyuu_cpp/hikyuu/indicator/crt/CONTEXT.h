/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

Indicator HKU_API CONTEXT(const Indicator& ind);
Indicator HKU_API CONTEXT();

KData HKU_API CONTEXT_K(const Indicator& ind);

}  // namespace hku