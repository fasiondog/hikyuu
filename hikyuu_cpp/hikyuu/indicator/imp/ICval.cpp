/*
 * ConstantValue.cpp
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
 */

#include "ICval.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICval)
#endif

namespace hku {

ICval::ICval() : IndicatorImp("CVAL", 1) {
    setParam<double>("value", 0.0);
    setParam<int>("discard", 0);
}

ICval::ICval(double value, size_t discard) : IndicatorImp("CVAL", 1) {
    setParam<double>("value", value);
    setParam<int>("discard", discard);
}

ICval::~ICval() {}

void ICval::_checkParam(const string& name) const {
    if ("discard" == name) {
        HKU_ASSERT(getParam<int>("discard") >= 0);
    }
}

void ICval::_calculate(const Indicator& data) {
    double value = getParam<double>("value");
    int discard = getParam<int>("discard");

    size_t total = 0;
    if (isLeaf()) {
        // 叶子节点
        KData k = getContext();
        if (k.getStock().isNull()) {
            _readyBuffer(1, 1);
            if (discard < 1) {
                m_discard = 0;
                _set(value, 0, 0);
            } else {
                m_discard = 1;
            }
            return;
        }

        total = k.size();
        if (0 == total) {
            return;
        }

        _readyBuffer(total, 1);

    } else {
        // 非叶子节点
        total = data.size();
        discard = data.discard() > discard ? data.discard() : discard;
    }

    m_discard = discard > total ? total : discard;

    auto* dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        dst[i] = value;
    }
}

Indicator HKU_API CVAL(double value, size_t discard) {
    return make_shared<ICval>(value, discard)->calculate();
}

Indicator HKU_API CVAL(const Indicator& ind, double value, int discard) {
    IndicatorImpPtr p = make_shared<ICval>();
    p->setParam<double>("value", value);
    p->setParam<int>("discard", discard);
    return Indicator(p)(ind);
}

} /* namespace hku */
