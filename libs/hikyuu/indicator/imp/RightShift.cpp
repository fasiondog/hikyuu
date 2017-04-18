/*
 * RightShift.cpp
 *
 *  Created on: 2015年3月21日
 *      Author: fasiondog
 */

#include "RightShift.h"

namespace hku {

RightShift::RightShift(): IndicatorImp("REF", 1) {
    setParam<int>("n", 1);
}

RightShift::~RightShift() {

}

bool RightShift::check() {
    int n = getParam<int>("n");
    if (n < 0) {
        HKU_ERROR("Invalid param! (n>=0) "
                  << m_params << " [RightShift::RightShift]");
        return false;
    }

    return true;
}

void RightShift::_calculate(const Indicator& data)  {
    size_t total = data.size();
    int n = getParam<int>("n");

    m_discard = data.discard() + n;
    for (size_t i = m_discard; i < total; ++i) {
        _set(data[i-n], i);
    }
}

Indicator HKU_API REF(int n) {
    IndicatorImpPtr p = make_shared<RightShift>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API REF(const Indicator& ind, int n) {
    IndicatorImpPtr p = make_shared<RightShift>();
    p->setParam<int>("n", n);
    p->calculate(ind);
    return Indicator(p);
}

} /* namespace hku */
