/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "NormMinMax.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NormMinMax)
#endif

namespace hku {

NormMinMax::NormMinMax() : NormalizeBase("NORM_MinMax") {}

NormMinMax::~NormMinMax() {}

PriceList NormMinMax::normalize(const PriceList& data) {
    PriceList result(data.size());
    HKU_IF_RETURN(data.empty(), result);

    price_t min_value = data.front();
    price_t max_value = data.front();
    for (size_t i = 1; i < data.size(); ++i) {
        if (!std::isnan(data[i])) {
            if (data[i] < min_value) {
                min_value = data[i];
            } else if (data[i] > max_value) {
                max_value = data[i];
            }
        }
    }

    if (min_value == max_value) {
        std::copy(data.begin(), data.end(), result.begin());
        return result;
    }

    price_t delta = max_value - min_value;
    for (size_t i = 0; i < data.size(); ++i) {
        if (std::isnan(data[i])) {
            result[i] = Null<price_t>();
        } else {
            result[i] = (data[i] - min_value) / delta;
        }
    }

    return result;
}

NormPtr HKU_API NORM_MinMax() {
    return std::make_shared<NormMinMax>();
}

}  // namespace hku