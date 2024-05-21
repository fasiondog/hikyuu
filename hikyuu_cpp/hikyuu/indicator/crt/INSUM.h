/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

Indicator HKU_API INSUM(const Block& block, const KQuery& query, const Indicator& ind, int mode);

Indicator HKU_API INSUM(const string& category, const string& name, const KQuery& query,
                        const Indicator& ind, int mode);

}  // namespace hku