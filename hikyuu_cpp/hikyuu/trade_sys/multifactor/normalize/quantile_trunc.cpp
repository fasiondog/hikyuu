/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-15
 *      Author: fasiondog
 */

#include "quantile_trunc.h"

namespace hku {

// 替换掉分位数范围外数值
PriceList quantile_trunc(const PriceList &src, double quantile_min, double quantile_max) {
    size_t total = src.size();
    PriceList ret(total);
    if (quantile_min == 0.0 && quantile_max == 1.0) {
        std::copy(src.begin(), src.end(), ret.begin());
        return ret;
    }

    PriceList tmp;
    tmp.reserve(total);
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(src[i])) {
            tmp.push_back(src[i]);
        }
    }
    std::sort(tmp.begin(), tmp.end());
    if (tmp.empty()) {
        for (size_t i = 0; i < total; i++) {
            ret[i] = Null<price_t>();
        }
        return ret;
    }

    auto down_limit = get_quantile(tmp, quantile_min);
    auto up_limit = get_quantile(tmp, quantile_max);
    // HKU_INFO("quantile_min: {}, quantile_max: {}", down_limit, up_limit);
    for (size_t i = 0; i < total; i++) {
        if (src[i] > up_limit) {
            ret[i] = up_limit;
        } else if (src[i] < down_limit) {
            ret[i] = down_limit;
        } else {
            ret[i] = src[i];
        }
    }

    return ret;
}

}  // namespace hku