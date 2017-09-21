/*
 * ConstantValue.cpp
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
 */

#include <hikyuu/indicator/imp/ConstantValue.h>

namespace hku {

ConstantValue::ConstantValue() : IndicatorImp("CVAL", 1) {
    setParam<double>("value", 0.0);
}

ConstantValue::ConstantValue(double value, size_t len, size_t discard)
:IndicatorImp("CVAL", 1) {
    m_discard = discard > len ? len : discard;
    setParam<double>("value", value);

    if (len == 0)
        return;

    _readyBuffer(len, m_result_num);
    for (size_t i = m_discard; i < len; ++i) {
        _set(value, i);
    }
}

ConstantValue::~ConstantValue() {

}

bool ConstantValue::check() {
    return true;
}

void ConstantValue::_calculate(const Indicator& data) {
    size_t total = data.size();

    double value = getParam<double>("value");
    m_discard = data.discard();

    for (size_t i = m_discard; i < total; ++i) {
        _set(value, i, 0);
    }
}

Indicator HKU_API CVAL(double value, size_t len, size_t discard) {
    IndicatorImpPtr p = make_shared<ConstantValue>(value, len, discard);
    return Indicator(p);
}

Indicator HKU_API CVAL(const Indicator& ind, double value) {
    IndicatorImpPtr p = make_shared<ConstantValue>();
    p->setParam<double>("value", value);
    p->calculate(ind);
    return Indicator(p);
}

} /* namespace hku */
