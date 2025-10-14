/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#include "NormZScore.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::NormZScore)
#endif

namespace hku {

NormZScore::NormZScore() : NormalizeBase("NORM_Zscore") {
    setParam<double>("nsigma", 3);
    setParam<bool>("out-extreme", false);
    setParam<bool>("recursive", false);
}

NormZScore::NormZScore(bool outExtreme, double nsigma, bool recursive)
: NormalizeBase("NORM_Zscore") {
    setParam<double>("nsigma", nsigma);
    setParam<bool>("out-extreme", outExtreme);
    setParam<bool>("recursive", recursive);
}

NormZScore::~NormZScore() {}

void NormZScore::_checkParam(const string& name) const {
    if ("nsigma" == name) {
        HKU_ASSERT(getParam<double>("nsigma") > 0.);
    }
}

PriceList NormZScore::normalize(const PriceList& data) {
    size_t total = data.size();
    PriceList ret(total, Null<price_t>());
    if (total <= 1) {
        std::copy(data.begin(), data.end(), ret.begin());
        return ret;
    }

    bool outExtreme = getParam<bool>("out-extreme");
    double nsigma = getParam<double>("nsigma");
    bool recursive = getParam<bool>("recursive");

    price_t sum = 0.0;
    size_t count = 0;
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(data[i])) {
            sum += data[i];
            count++;
        }
    }

    if (count <= 1) {
        std::copy(data.begin(), data.end(), ret.begin());
        return ret;
    }

    price_t mean = sum / count;

    PriceList tmp(total, Null<price_t>());
    sum = 0.0;
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(data[i])) {
            tmp[i] = data[i] - mean;
            sum += tmp[i] * tmp[i];
        }
    }

    price_t sigma = std::sqrt(sum / (count - 1));
    for (size_t i = 0; i < total; i++) {
        if (!std::isnan(data[i])) {
            ret[i] = (data[i] - mean) / sigma;
        }
    }

    if (outExtreme) {
        price_t ulimit = nsigma;
        price_t llimit = -nsigma;

        bool found = false;
        for (size_t i = 0; i < total; i++) {
            if (!std::isnan(ret[i])) {
                if (ret[i] > ulimit) {
                    ret[i] = ulimit;
                    found = true;
                } else if (ret[i] < llimit) {
                    ret[i] = llimit;
                    found = true;
                }
            }
        }

        if (found && recursive) {
            ret = normalize(ret);
        }
    }

    return ret;
}

NormPtr HKU_API NORM_Zscore(bool outExtreme, double nsigma, bool recursive) {
    return std::make_shared<NormZScore>(outExtreme, nsigma, recursive);
}

}  // namespace hku