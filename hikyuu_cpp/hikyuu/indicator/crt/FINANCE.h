/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-13
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

Indicator HKU_API FINANCE(int field_ix);
Indicator HKU_API FINANCE(const KData& k, int field_ix);

Indicator HKU_API FINANCE(const string& field_name);
Indicator HKU_API FINANCE(const KData& k, const string& field_name);

}  // namespace hku