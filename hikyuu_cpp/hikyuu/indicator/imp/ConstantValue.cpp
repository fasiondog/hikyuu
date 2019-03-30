/*
 * ConstantValue.cpp
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
 */

#include "ConstantValue.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ConstantValue)
#endif


namespace hku {

ConstantValue::ConstantValue() : IndicatorImp("CVAL", 1) {
    setParam<double>("value", 0.0);
    setParam<int>("discard", 0);
}

ConstantValue::ConstantValue(double value, size_t discard)
:IndicatorImp("CVAL", 1) {
    setParam<double>("value", value);
    setParam<int>("discard", discard);
}

ConstantValue::~ConstantValue() {

}

bool ConstantValue::check() {
    return getParam<int>("discard") < 0 ? false : true;
}

void ConstantValue::_calculate(const Indicator& data) {
    double value = getParam<double>("value");
    int discard = getParam<int>("discard");

    size_t total = 0;
    if (isLeaf()) {
        //叶子节点
        KData k = getContext();
        total = k.size();
        if (0 == total) {
            return;
        }
        _readyBuffer(total, 1);
    
    } else {
        //非叶子节点
        total = data.size();
        discard = data.discard() > discard ? data.discard() : discard;
    }

    m_discard = discard > total ? total : discard;

    for (size_t i = m_discard; i < total; ++i) {
        _set(value, i, 0);
    }
}

Indicator HKU_API CVAL(double value, size_t discard) {
    return make_shared<ConstantValue>(value, discard)->calculate();
}

Indicator HKU_API CVAL(const Indicator& ind, double value, int discard) {
    IndicatorImpPtr p = make_shared<ConstantValue>();
    p->setParam<double>("value", value);
    p->setParam<int>("discard", discard);
    return Indicator(p)(ind);
}

} /* namespace hku */
