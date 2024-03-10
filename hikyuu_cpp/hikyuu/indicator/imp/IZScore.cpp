/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "IZScore.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IZScore)
#endif

namespace hku {

IZScore::IZScore() : IndicatorImp("ZSCORE", 1) {
    setParam<double>("nsigma", 3);
    setParam<bool>("out-extreme", false);
    setParam<bool>("recursive", false);
}

IZScore::IZScore(bool outExtreme, double nsigma, bool recursive) : IndicatorImp("ZSCORE", 1) {
    setParam<double>("nsigma", nsigma);
    setParam<bool>("out-extreme", outExtreme);
    setParam<bool>("recursive", recursive);
}

IZScore::~IZScore() {}

bool IZScore::check() {
    return getParam<double>("nsigma") > 0.;
}

static void normalize(IndicatorImp::value_t *dst, Indicator::value_t const *src, size_t total,
                      bool outExtreme, double nsigma, bool recursive) {
    IndicatorImp::value_t sum = 0.0;
    for (size_t i = 0; i < total; i++) {
        sum += src[i];
    }
    IndicatorImp::value_t mean = sum / total;

    vector<IndicatorImp::value_t> tmp(total);
    sum = 0.0;
    for (size_t i = 0; i < total; i++) {
        tmp[i] = src[i] - mean;
        sum += tmp[i] * tmp[i];
    }
    IndicatorImp::value_t sigma = std::sqrt(sum / (total - 1));
    for (size_t i = 0; i < total; i++) {
        dst[i] = (src[i] - mean) / sigma;
    }

    if (outExtreme) {
        IndicatorImp::value_t ulimit = mean + nsigma * sigma;
        IndicatorImp::value_t llimit = mean - nsigma * sigma;
        bool found = false;
        for (size_t i = 0; i < total; i++) {
            if (dst[i] > ulimit) {
                dst[i] = ulimit;
                found = true;
            } else if (dst[i] < llimit) {
                dst[i] = llimit;
                found = true;
            }
        }

        if (found && recursive) {
            normalize(dst, src, total, outExtreme, nsigma, recursive);
        }
    }
}

void IZScore::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard + 1 >= total) {
        m_discard = total;
        return;
    }

    double nsigma = getParam<double>("nsigma");
    bool outExtreme = getParam<bool>("out-extreme");
    bool recursive = getParam<bool>("recursive");
    auto const *src = data.data() + m_discard;
    auto *dst = this->data() + m_discard;
    normalize(dst, src, total - m_discard, outExtreme, nsigma, recursive);
}

Indicator HKU_API ZSCORE(bool outExtreme, double nsigma, bool recursive) {
    return Indicator(make_shared<IZScore>(outExtreme, nsigma, recursive));
}

}  // namespace hku