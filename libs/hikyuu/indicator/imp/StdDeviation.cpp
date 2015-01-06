/*
 * StdDeviation.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "StdDeviation.h"
#include "../crt/MA.h"

namespace hku {

StdDeviation::StdDeviation(): IndicatorImp() {

}

StdDeviation::StdDeviation(int n)
: IndicatorImp(n - 1, 1) {
    addParam<int>("n", n);

    if (n < 2) {
        HKU_ERROR("Invalid param[n] ! (n >= 2) " << m_params
                << " [StdDeviation::StdDeviation]");
        return;
    }
}

StdDeviation::StdDeviation(const Indicator& data, int n)
: IndicatorImp(data, n - 1, 1) {
    addParam<int>("n", n);

    if (n < 2) {
        HKU_ERROR("Invalid param[n] ! (n >= 2) " << m_params
                << " [StdDeviation::StdDeviation]");
        return;
    }

    size_t total = data.size();
    Indicator ma = MA(data, n);
    size_t N = n - 1;
    for (size_t i = discard(); i < total; ++i) {
        price_t mean = ma[i];
        price_t sum = 0.0;
        for (size_t j = i + 1 - n; j <= i; ++j) {
            sum += std::pow(data[j] - mean, 2);
        }
        _set(std::sqrt(sum/N), i);
    }
}

StdDeviation::~StdDeviation() {

}

string StdDeviation::name() const {
    return "STD";
}

IndicatorImpPtr StdDeviation::operator()(const Indicator& ind) {
    return IndicatorImpPtr(new StdDeviation(ind, getParam<int>("n")));
}

Indicator HKU_API STDEV(int n) {
    return Indicator(IndicatorImpPtr(new StdDeviation(n)));
}

Indicator HKU_API STDEV(const Indicator& data, int n) {
    return Indicator(IndicatorImpPtr(new StdDeviation(data, n)));
}

} /* namespace hku */
