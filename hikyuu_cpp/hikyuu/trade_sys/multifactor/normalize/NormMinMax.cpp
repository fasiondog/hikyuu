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
    PriceList result(data.size(), Null<price_t>());
    HKU_IF_RETURN(data.empty(), result);

    size_t first = 0;
    while (first < data.size() && std::isnan(data[first])) {
        ++first;
    }
    HKU_IF_RETURN(first == data.size(), result);

    price_t min_value = data[first];
    price_t max_value = data[first];
    for (size_t i = first + 1; i < data.size(); ++i) {
        if (!std::isnan(data[i])) {
            if (data[i] < min_value) {
                min_value = data[i];
            } else if (data[i] > max_value) {
                max_value = data[i];
            }
        }
    }

    if (min_value == max_value) {
        // A zero-range cross-section has no dispersion to scale: map the valid positions to the
        // constant 0 (rather than leaking the raw magnitudes); being constant it leaves the
        // cross-sectional ordering unchanged
        for (size_t i = 0; i < data.size(); ++i) {
            if (!std::isnan(data[i])) {
                result[i] = 0.0;
            }
        }
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