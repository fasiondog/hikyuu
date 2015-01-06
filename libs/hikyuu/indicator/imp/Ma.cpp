/*
 * Ma.cpp
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */

#include "Ma.h"

namespace hku {

Ma::Ma(int n): IndicatorImp(n-1, 1) {
    addParam<int>("n", n);
    if (n < 1) {
        HKU_ERROR("Invalid param! (n >= 1) [Ma::Ma]");
        return;
    }
}

Ma::Ma(const Indicator& indicator, int n)
: IndicatorImp(indicator, n - 1, 1) {
    addParam<int>("n", n);

    if (n < 1) {
        HKU_ERROR("Invalid param! (n >= 1) [Ma::Ma]");
        return;
    }

    size_t total = indicator.size();
    if (total <= discard()) {
        return;
    }

    size_t startPos = discard();
    price_t sum = 0.0;
    for (size_t i = startPos + 1 - n; i <= startPos; ++i) {
        sum += indicator[i];
    }

    _set(sum/n, startPos);

    for(size_t i = startPos + 1; i < total; ++i) {
        sum = indicator[i] + sum - indicator[i-n];
        _set(sum/n, i);
    }
}


Ma::~Ma() {

}


string Ma::name() const {
    return "MA";
}


IndicatorImpPtr Ma::operator()(const Indicator& ind) {
    return IndicatorImpPtr(new Ma(ind, getParam<int>("n")));
}

Indicator HKU_API MA(int n) {
    return Indicator(IndicatorImpPtr(new Ma(n)));
}

Indicator HKU_API MA(const Indicator& indicator, int n){
    return Indicator(IndicatorImpPtr(new Ma(indicator, n)));
}

} /* namespace hku */
