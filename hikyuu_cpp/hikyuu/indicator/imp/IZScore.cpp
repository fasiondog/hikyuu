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

void IZScore::_checkParam(const string &name) const {
    if ("nsigma" == name) {
        HKU_ASSERT(getParam<double>("nsigma") > 0.);
    }
}

static void normalize(IndicatorImp::value_t *dst, Indicator::value_t const *src, size_t total,
                      bool outExtreme, double nsigma, bool recursive) {
    IndicatorImp::value_t sum = 0.0;
    size_t count = 0;
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(src[i])) {
            sum += src[i];
            count++;
        }
    }

    HKU_IF_RETURN(count <= 1, void());

    IndicatorImp::value_t mean = sum / count;

    vector<IndicatorImp::value_t> tmp(total, Null<IndicatorImp::value_t>());
    sum = 0.0;
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(src[i])) {
            tmp[i] = src[i] - mean;
            sum += tmp[i] * tmp[i];
        }
    }

    IndicatorImp::value_t sigma = std::sqrt(sum / (count - 1));
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(src[i])) {
            dst[i] = (src[i] - mean) / sigma;
        }
    }

    if (outExtreme) {
        IndicatorImp::value_t ulimit = nsigma;
        IndicatorImp::value_t llimit = -nsigma;

        bool found = false;
        for (size_t i = 0; i < total; i++) {
            if (!std::isnan(dst[i])) {
                if (dst[i] > ulimit) {
                    dst[i] = ulimit;
                    found = true;
                } else if (dst[i] < llimit) {
                    dst[i] = llimit;
                    found = true;
                }
            }
        }

        if (found && recursive) {
            normalize(dst, dst, total, outExtreme, nsigma, recursive);
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

    for (size_t i = m_discard; i < total; i++) {
        if (!std::isnan(dst[i])) {
            m_discard = i;
            break;
        }
    }
}

Indicator HKU_API ZSCORE(bool outExtreme, double nsigma, bool recursive) {
    return Indicator(make_shared<IZScore>(outExtreme, nsigma, recursive));
}

}  // namespace hku