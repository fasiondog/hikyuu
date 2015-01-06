/*
 * Ama.cpp
 *
 *  Created on: 2013-4-7
 *      Author: fasiondog
 */

#include <cmath>
#include "Ama.h"

namespace hku {

Ama::Ama(int n, int fast_n, int slow_n)
: IndicatorImp(n, 2) {
    addParam<int>("n", n);
    addParam<int>("fast_n", fast_n);
    addParam<int>("slow_n", slow_n);

    if (n < 1 || fast_n < 0 || slow_n < 0) {
        HKU_ERROR("Invalid param! (n>=1, fast_n>0, slow_n>0) "
                  << m_params << " [Ama::Ama]");
        return;
    }
}

Ama::Ama(const Indicator& data, int n, int fast_n, int slow_n)
: IndicatorImp(data, n, 2) {
    addParam<int>("n", n);
    addParam<int>("fast_n", fast_n);
    addParam<int>("slow_n", slow_n);

    if (n < 1 || fast_n < 0 || slow_n < 0) {
        HKU_ERROR("Invalid param! (n>=1, fast_n>0, slow_n>0) "
                  << m_params << " [Ama::Ama]");
        return;
    }

    size_t total = data.size();
    if (total <= discard()) {
        return;
    }

    size_t start = discard();
    if (start < (size_t)n) {
        HKU_ERROR("Some error! [Ama::Ama]");
        return;
    }

    price_t fastest = 2.0 / (fast_n + 1);
    price_t slowest = 2.0 / (slow_n + 1);
    price_t delta = fastest - slowest;

    price_t prevol = 0.0, vol = 0.0, er = 0.0, c = 0.0;
    for (size_t i = start + 1 - n; i <= start; i++){
        vol += std::abs(data[i] - data[i-1]);
    }
    er = (vol == 0.0) ? 0.0 : (data[start] - data[start-n]) / vol;
    c = std::pow((std::abs(er) * delta + slowest), 2);
    price_t ama = data[start - 1];
    ama += c * (data[start] - ama);
    prevol = vol;
    _set(ama, start, 0);
    _set(er, start, 1);

    for (size_t i = start + 1; i < total; ++i) {
        vol = prevol + std::abs(data[i] - data[i-1])
                     - std::abs(data[i-n] - data[i-n-1]);
        er = (vol == 0.0) ? 0.0 : (data[i] - data[i-n]) / vol;
        c = std::pow((std::abs(er) * delta + slowest), 2);
        ama += c * (data[i] - ama);
        prevol = vol;
        _set(ama, i, 0);
        _set(er, i, 1);
    }
}

Ama::~Ama() {

}

string Ama::name() const {
    return "AMA";
}

IndicatorImpPtr Ama::operator()(const Indicator& ind) {
    return IndicatorImpPtr(new Ama(ind, getParam<int>("n"),
            getParam<int>("fast_n"), getParam<int>("slow_n")));
}

Indicator HKU_API AMA(int n, int fast_n, int slow_n) {
    return Indicator(IndicatorImpPtr(new Ama(n, fast_n, slow_n)));
}

Indicator HKU_API AMA(const Indicator& indicator,
        int n, int fast_n, int slow_n) {
    return Indicator(IndicatorImpPtr(new Ama(indicator, n, fast_n, slow_n)));
}

} /* namespace hku */
