/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-16
 *      Author: fasiondog
 */

#include "IIsLastBar.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIsLastBar)
#endif

namespace hku {

IIsLastBar::IIsLastBar() : IndicatorImp("ISLASTBAR", 1) {}

IIsLastBar::IIsLastBar(const KData &k) : IndicatorImp("ISLASTBAR", 1) {
    setParam<KData>("kdata", k);
    IIsLastBar::_calculate(Indicator());
}

IIsLastBar::~IIsLastBar() {}

void IIsLastBar::_calculate(const Indicator &data) {
    size_t total = data.size();
    if (isLeaf()) {
        KData k = getContext();
        total = k.size();
        _readyBuffer(total, 1);
        if (total >= 1) {
            auto *dst = this->data();
            memset(dst, 0, sizeof(value_t) * (total - 1));
            dst[total - 1] = 1.;
        }
        return;
    }

    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t len = total - m_discard;
    HKU_IF_RETURN(len == 0, void());

    auto *dst = this->data() + m_discard;
    memset(dst, 0, sizeof(value_t) * len);
    dst[total - 1] = 1.;
}

Indicator HKU_API ISLASTBAR() {
    return Indicator(make_shared<IIsLastBar>());
}

Indicator HKU_API ISLASTBAR(const KData &kdata) {
    return Indicator(make_shared<IIsLastBar>(kdata));
}

} /* namespace hku */
