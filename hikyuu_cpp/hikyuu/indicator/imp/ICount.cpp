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
    if (0 == total) {
        m_discard = 0;
        return;
    }

    if (data.discard() >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");

    if (0 == n) {
        m_discard = data.discard();
        int count = 0;
        for (size_t i = m_discard; i < total; ++i) {
            if (data[i] != 0) {
                count++;
            }
            _set(count, i);
        }
        return;
    }

    m_discard = data.discard() + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t startPos = data.discard();
    int sum = 0;
    size_t first_end = startPos + n >= total ? total : startPos + n;
    for (size_t i = startPos; i < first_end; ++i) {
        if (data[i] != 0) {
            sum++;
        }
    }

    if (first_end >= 1) {
        _set(sum, first_end - 1);
    }
    
    for (size_t i = first_end; i < total; ++i) {
        if (data[i] != 0) {
            sum++;
        }
        if (data[i-n] != 0) {
            sum--;
        }
        _set(sum, i);
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
