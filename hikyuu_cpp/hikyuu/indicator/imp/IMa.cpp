/*
 * IMa.cpp
 *
 *  Created on: 2013-2-10
 *      Author: fasiondog
 */

#include "IMa.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMa)
#endif


namespace hku {

IMa::IMa(): IndicatorImp("MA", 1) {
    setParam<int>("n", 22);
}

IMa::~IMa() {

}

bool IMa::check() {
    int n = getParam<int>("n");
    if (n < 1) {
        HKU_ERROR("Invalid param! (n >= 1) {}", m_params);
        return false;
    }

    return true;
}

void IMa::_calculate(const Indicator& indicator) {
    size_t total = indicator.size();
    m_discard = indicator.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    int n = getParam<int>("n");
    size_t startPos = m_discard;
    price_t sum = 0.0;
    size_t count = 1;
    size_t first_end = startPos + n >= total ? total : startPos + n;
    for (size_t i = startPos; i < first_end; ++i) {
        sum += indicator[i];
        _set(sum/count++, i);
    }

    for (size_t i = first_end; i < total; ++i) {
        sum = indicator[i] + sum - indicator[i-n];
        _set(sum/n, i);
    }
}


Indicator HKU_API MA(int n) {
    IndicatorImpPtr p = make_shared<IMa>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API MA(const Indicator& ind, int n){
    return MA(n)(ind);
}

} /* namespace hku */
