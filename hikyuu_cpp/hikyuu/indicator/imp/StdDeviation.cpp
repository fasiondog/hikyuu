/*
 * StdDeviation.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "StdDeviation.h"
#include "../crt/MA.h"

namespace hku {

StdDeviation::StdDeviation(): IndicatorImp("STD", 1) {
    setParam<int>("n", 10);
}

StdDeviation::~StdDeviation() {

}

bool StdDeviation::check() {
    int n = getParam<int>("n");
    if (n < 2) {
        HKU_ERROR("Invalid param[n] ! (n >= 2) " << m_params
                << " [StdDeviation::calculate]");
        return false;
    }

    return true;
}

void StdDeviation::_calculate(const Indicator& data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    m_discard = data.discard() + n - 1;

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


Indicator HKU_API STDEV(int n) {
    IndicatorImpPtr p = make_shared<StdDeviation>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API STDEV(const Indicator& data, int n) {
    IndicatorImpPtr p = make_shared<StdDeviation>();
    p->setParam<int>("n", n);
    p->calculate(data);
    return Indicator(p);
}

} /* namespace hku */
