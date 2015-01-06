/*
 * SaftyLoss.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "SaftyLoss.h"

namespace hku {

SaftyLoss::SaftyLoss(int n1, int n2, double p)
:IndicatorImp(n1 + n2 - 2, 1) {
    addParam<int>("n1", n1);
    addParam<int>("n2", n2);
    addParam<double>("p", p);

    if (n1 < 2) {
        HKU_ERROR("Invalid param[n1] must >= 2 ! [SaftyLoss::SaftyLoss]");
        return;
    }

    if (n2 < 1) {
        HKU_ERROR("Invalid param[n2] must >= 1 ! [SaftyLoss::SaftyLoss]");
        return;
    }
}

SaftyLoss::SaftyLoss(const Indicator& data, int n1, int n2, double p)
:IndicatorImp(data, n1 + n2 - 2, 1) {
    addParam<int>("n1", n1);
    addParam<int>("n2", n2);
    addParam<double>("p", p);

    if (n1 < 2) {
        HKU_ERROR("Invalid param[n1] must >= 2 ! [SaftyLoss::SaftyLoss]");
        return;
    }

    if (n2 < 1) {
        HKU_ERROR("Invalid param[n2] must >= 1 ! [SaftyLoss::SaftyLoss]");
        return;
    }

    size_t total = data.size();
    if (0 == total) {
        return;
    }

    price_t sum = 0.0;
    size_t num = 0;
    price_t result = 0.0;

    size_t start = discard();
    for (size_t i = start; i < total; ++i) {
        result = 0.0;
        for (size_t j = i + 1 - n2; j <= i; ++j) {
            sum = 0.0;
            num = 0;
            for (size_t k = j + 2 -n1; k <=j; ++k) {
                price_t pre = data[k-1];
                price_t cur = data[k];
                if (pre > cur) {
                    sum += pre - cur;
                    ++num;
                }
            }

            price_t temp = data[j];
            if (num != 0) {
                temp = temp - (p * sum / num);
            }

            if (temp > result) {
                result = temp;
            }
        }

        _set(result, i);
    }
}

SaftyLoss::~SaftyLoss() {

}

string SaftyLoss::name() const {
    return "SaftyLoss";
}

IndicatorImpPtr SaftyLoss::operator()(const Indicator& ind) {
    return IndicatorImpPtr(new SaftyLoss(ind, getParam<int>("n1"),
            getParam<int>("n2"), getParam<double>("p")));
}


Indicator HKU_API SAFTYLOSS(int n1, int n2, double p) {
    return Indicator(IndicatorImpPtr(new SaftyLoss(n1, n2, p)));
}


Indicator HKU_API SAFTYLOSS(const Indicator& data, int n1, int n2, double p) {
    return Indicator(IndicatorImpPtr(new SaftyLoss(data, n1, n2, p)));
}

} /* namespace hku */
