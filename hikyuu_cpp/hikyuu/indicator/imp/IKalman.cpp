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
    setParam<double>("q", 0.01);  // 过程噪声协方差
    setParam<double>("r", 0.1);   // 测量噪声协方差
}

IKalman::~IKalman() {}

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

    value_t x = src[m_discard];  // 状态估计值
    value_t p = 1.0;             // 估计误差协方差

    dst[m_discard] = x;
    for (size_t i = m_discard + 1; i < total; ++i) {
        p = p + q;
        value_t k = p / (p + r);
        x = x + k * (src[i] - x);
        p = (1 - k) * p;
        dst[i] = x;
    }
}

Indicator HKU_API KALMAN(double q, double r) {
    auto p = make_shared<IKalman>();
    p->setParam<double>("q", q);
    p->setParam<double>("r", r);
    return Indicator(p);
}

} /* namespace hku */
