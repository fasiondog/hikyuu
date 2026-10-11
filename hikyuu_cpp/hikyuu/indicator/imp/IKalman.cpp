/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-02
 *      Author: fasiondog
 */

#include "IKalman.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IKalman)
#endif

namespace hku {

IKalman::IKalman() : IndicatorImp("KALMAN", 1) {
    setParam<double>("q", 0.01);  // Process noise covariance
    setParam<double>("r", 0.1);   // Measurement noise covariance
}

IKalman::~IKalman() {}

void IKalman::_checkParam(const string &name) const {
    if ("q" == name) {
        HKU_ASSERT(getParam<double>("q") > 0.);
    } else if ("r" == name) {
        HKU_ASSERT(getParam<double>("r") > 0.);
    }
}

void IKalman::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    value_t q = getParam<double>("q");
    value_t r = getParam<double>("r");

    auto const *src = data.data();
    auto *dst = this->data();

    value_t x = src[m_discard];  // State estimate
    value_t p = 1.0;             // Estimation error covariance

    dst[m_discard] = x;
    for (size_t i = m_discard + 1; i < total; ++i) {
        if (!std::isfinite(x)) {
            // Not yet seeded (leading src was non-finite): wait for the first valid observation
            x = src[i];
            p = 1.0;
            dst[i] = x;
        } else if (!std::isfinite(src[i])) {
            // Missing observation: predict-only (uncertainty grows, state held)
            p = p + q;
            dst[i] = x;
        } else {
            p = p + q;
            value_t k = p / (p + r);
            x = x + k * (src[i] - x);
            p = (1 - k) * p;
            dst[i] = x;
        }
    }
}

Indicator HKU_API KALMAN(double q, double r) {
    auto p = make_shared<IKalman>();
    p->setParam<double>("q", q);
    p->setParam<double>("r", r);
    return Indicator(p);
}

} /* namespace hku */
