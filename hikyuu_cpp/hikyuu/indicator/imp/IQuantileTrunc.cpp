/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "IQuantileTrunc.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IQuantileTrunc)
#endif

namespace hku {

IQuantileTrunc::IQuantileTrunc() : IndicatorImp("QUANTILE_TRUNC", 1) {
    setParam<double>("quantile_min", 0.01);
    setParam<double>("quantile_max", 0.99);
}

IQuantileTrunc::IQuantileTrunc(double quantile_min, double quantile_max)
: IndicatorImp("QUANTILE_TRUNC", 1) {
    setParam<double>("quantile_min", quantile_min);
    setParam<double>("quantile_max", quantile_max);
}

IQuantileTrunc::~IQuantileTrunc() {}

void IQuantileTrunc::_checkParam(const string &name) const {
    if ("quantile_min" == name) {
        double quantile_min = getParam<double>("quantile_min");
        HKU_ASSERT(quantile_min >= 0.0 && quantile_min <= 1.0);
        if (haveParam("quantile_max")) {
            double quantile_max = getParam<double>("quantile_max");
            HKU_ASSERT(quantile_min < quantile_max);
        }
    } else if ("quantile_max" == name) {
        double quantile_max = getParam<double>("quantile_max");
        HKU_ASSERT(quantile_max >= 0.0 && quantile_max <= 1.0);
        if (haveParam("quantile_min")) {
            double quantile_min = getParam<double>("quantile_min");
            HKU_ASSERT(quantile_min < quantile_max);
        }
    }
}

// 替换掉分位数范围外数值
static void quantile_trunc(IndicatorImp::value_t *dst, Indicator::value_t const *src, size_t total,
                           double quantile_min, double quantile_max) {
    HKU_IF_RETURN(quantile_min == 0.0 && quantile_max == 1.0, void());

    std::vector<IndicatorImp::value_t> tmp;
    tmp.reserve(total);
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(src[i])) {
            tmp.push_back(src[i]);
        }
    }
    HKU_IF_RETURN(tmp.empty(), void());

    std::sort(tmp.begin(), tmp.end());

    auto down_limit = get_quantile(tmp, quantile_min);
    auto up_limit = get_quantile(tmp, quantile_max);
    HKU_INFO("quantile_min: {}, quantile_max: {}", down_limit, up_limit);
    for (size_t i = 0; i < total; i++) {
        if (src[i] > up_limit) {
            dst[i] = up_limit;
        } else if (src[i] < down_limit) {
            dst[i] = down_limit;
        } else {
            dst[i] = src[i];
        }
    }
}

void IQuantileTrunc::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard + 1 >= total) {
        m_discard = total;
        return;
    }

    auto const *src = data.data() + m_discard;
    auto *dst = this->data() + m_discard;
    quantile_trunc(dst, src, total - m_discard, getParam<double>("quantile_min"),
                   getParam<double>("quantile_max"));

    _update_discard();
}

Indicator HKU_API QUANTILE_TRUNC(double quantile_min, double quantile_max) {
    return Indicator(make_shared<IQuantileTrunc>(quantile_min, quantile_max));
}

}  // namespace hku