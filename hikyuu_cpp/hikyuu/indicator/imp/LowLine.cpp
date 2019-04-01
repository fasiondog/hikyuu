/*
 * LowLine.cpp
 *
 *  Created on: 2016年4月2日
 *      Author: fasiondog
 */

#include "LowLine.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::LowLine)
#endif


namespace hku {

LowLine::LowLine() : IndicatorImp("LLV", 1) {
    setParam<int>("n", 20);
}

LowLine::~LowLine() {

}

bool LowLine::check() {
    int n = getParam<int>("n");
    if (n < 0) {
        HKU_ERROR("Invalid param[n] ! (n >= 0) " << m_params
                << " [LowLine::calculate]");
        return false;
    }

    return true;
}

void LowLine::_calculate(const Indicator& data) {
    size_t total = data.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    int n = getParam<int>("n");
    if (0 == n) {
        m_discard = data.discard();
        if (m_discard >= total) {
            return ;
        }

        price_t min = data[0];
        _set(min, 0);
        for (size_t i = m_discard + 1; i < total; i++) {
            if (data[i] < min) {
                min = data[i];
            }
            _set(min, i);
        }
        return;
    }

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

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
    IndicatorImpPtr p = make_shared<LowLine>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API LLV(const Indicator& ind, int n =20) {
    return LLV(n)(ind);
}

} /* namespace hku */
