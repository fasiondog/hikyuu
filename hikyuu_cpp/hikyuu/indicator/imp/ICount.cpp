/*
 * Icount.cpp
 *
 *  Created on: 2019年3月25日
 *      Author: fasiondog
 */

#include "ICount.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICount)
#endif


namespace hku {

ICount::ICount() : IndicatorImp("COUNT", 1) {
    setParam<int>("n", 20);
}

ICount::~ICount() {

}

bool ICount::check() {
    int n = getParam<int>("n");
    if (n < 0) {
        HKU_ERROR("Invalid param[n] ! (n >= 0) " << m_params
                << " [ICount::calculate]");
        return false;
    }

    return true;
}

void ICount::_calculate(const Indicator& data) {
    size_t total = data.size();
    int n = getParam<int>("n");

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    for (size_t i = m_discard; i < total; ++i) {
        int count = 0;
        for (size_t j = i + 1 - n; j <= i; ++j) {
            if (data[j] > 0) {
                count++;
            }
        }
        _set(count, i);
    }
}


Indicator HKU_API COUNT(int n) {
    IndicatorImpPtr p = make_shared<ICount>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API COUNT(const Indicator& ind, int n) {
    return COUNT(n)(ind);
}

} /* namespace hku */
