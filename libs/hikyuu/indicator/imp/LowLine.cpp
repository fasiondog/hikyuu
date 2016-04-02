/*
 * LowLine.cpp
 *
 *  Created on: 2016年4月2日
 *      Author: Administrator
 */

#include "LowLine.h"

namespace hku {

LowLine::LowLine() : IndicatorImp("LLV") {
    setParam<int>("n", 20);
}

LowLine::~LowLine() {

}

void LowLine::calculate(const Indicator& data) {
    size_t total = data.size();
    _readyBuffer(total, 1);

    int n = getParam<int>("n");
    if (n < 1) {
        HKU_ERROR("Invalid param[n] ! (n >= 1) " << m_params
                << " [HighLine::calculate]");
        return;
    }

    m_discard = data.discard() + n - 1;

    size_t pos = m_discard + 1 - n;
    price_t min = 0;
    for (size_t i = m_discard; i < total; ++i) {
        size_t j = i + 1 - n;
        if (pos > j) {
            j = pos;
        } else {
            min = data[j];
        }
        for (; j <= i; ++j) {
            if (data[j] < min) {
                min = data[j];
                pos = j;
            }
        }
        _set(min, i);
    }
}

Indicator HKU_API LLV(int n =20) {
    IndicatorImpPtr p(new LowLine);
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API LLV(const Indicator& ind, int n =20) {
    IndicatorImpPtr p(new LowLine);
    p->setParam<int>("n", n);
    p->calculate(ind);
    return Indicator(p);
}

} /* namespace hku */
