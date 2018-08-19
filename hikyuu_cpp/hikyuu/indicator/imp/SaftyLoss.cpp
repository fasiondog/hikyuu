/*
 * SaftyLoss.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "SaftyLoss.h"

namespace hku {

SaftyLoss::SaftyLoss():IndicatorImp("SAFTYLOSS", 1) {
    setParam<int>("n1", 10);
    setParam<int>("n2", 3);
    setParam<double>("p", 2.0);
}


SaftyLoss::~SaftyLoss() {

}

bool SaftyLoss::check() {
    int n1 = getParam<int>("n1");
    int n2 = getParam<int>("n2");
    double p = getParam<double>("p");

    if (n1 < 2) {
        HKU_ERROR("Invalid param[n1] must >= 2 ! [SaftyLoss::SaftyLoss]");
        return false;
    }

    if (n2 < 1) {
        HKU_ERROR("Invalid param[n2] must >= 1 ! [SaftyLoss::SaftyLoss]");
        return false;
    }

    return true;
}

void SaftyLoss::_calculate(const Indicator& data) {
    size_t total = data.size();
    _readyBuffer(total, 1);

    int n1 = getParam<int>("n1");
    int n2 = getParam<int>("n2");
    double p = getParam<double>("p");

    m_discard = data.discard() + n1 + n2 - 2;
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


Indicator HKU_API SAFTYLOSS(int n1, int n2, double p) {
    IndicatorImpPtr result = make_shared<SaftyLoss>();
    result->setParam<int>("n1", n1);
    result->setParam<int>("n2", n2);
    result->setParam<double>("p", p);
    return Indicator(result);
}


Indicator HKU_API SAFTYLOSS(const Indicator& data, int n1, int n2, double p) {
    IndicatorImpPtr result = make_shared<SaftyLoss>();
    result->setParam<int>("n1", n1);
    result->setParam<int>("n2", n2);
    result->setParam<double>("p", p);
    result->calculate(data);
    return Indicator(result);
}

} /* namespace hku */
