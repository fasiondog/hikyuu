/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "NormNothing.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NormNothing)
#endif

namespace hku {

NormNothing::NormNothing() : NormalizeBase("NORM_NOTHING") {}

NormNothing::~NormNothing() {}

PriceList NormNothing::normalize(const PriceList& data) {
    PriceList result(data.size());
    std::copy(data.begin(), data.end(), result.begin());
    return result;
}

NormPtr HKU_API NORM_NOTHIN() {
    return std::make_shared<NormNothing>();
}

}  // namespace hku