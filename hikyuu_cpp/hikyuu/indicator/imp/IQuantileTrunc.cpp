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
    setParam<int>("n", 60);
    setParam<double>("quantile_min", 0.01);
    setParam<double>("quantile_max", 0.99);
}

IQuantileTrunc::~IQuantileTrunc() {}

void IQuantileTrunc::_checkParam(const string &name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") > 0);
    } else if ("quantile_min" == name) {
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
static Indicator::value_t quantile_trunc(Indicator::value_t const *src, size_t total,
                                         double quantile_min, double quantile_max) {
    HKU_IF_RETURN(quantile_min == 0.0 && quantile_max == 1.0, src[total - 1]);

    std::vector<IndicatorImp::value_t> tmp;
    tmp.reserve(total);
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(src[i])) {
            tmp.push_back(src[i]);
        }
    }
    HKU_IF_RETURN(tmp.empty(), src[total - 1]);

    std::sort(tmp.begin(), tmp.end());

    auto down_limit = get_quantile(tmp, quantile_min);
    auto up_limit = get_quantile(tmp, quantile_max);
    if (src[total - 1] > up_limit) {
        return up_limit;
    } else if (src[total - 1] < down_limit) {
        return down_limit;
    }
    return src[total - 1];
}

void IQuantileTrunc::_calculate(const Indicator &data) {
    size_t total = data.size();
    int n = getParam<int>("n");
    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    double quantile_min = getParam<double>("quantile_min");
    double quantile_max = getParam<double>("quantile_max");
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; i++) {
        auto const *src = data.data() + 1 + i - n;
        dst[i] = quantile_trunc(src, n, quantile_min, quantile_max);
    }
}

Indicator HKU_API QUANTILE_TRUNC(int n, double quantile_min, double quantile_max) {
    auto p = make_shared<IQuantileTrunc>();
    p->setParam<int>("n", n);
    p->setParam<double>("quantile_min", quantile_min);
    p->setParam<double>("quantile_max", quantile_max);
    return Indicator(p);
}

}  // namespace hku