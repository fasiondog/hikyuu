/*
 * IAd.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#include "IAd.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAd)
#endif

namespace hku {

IAd::IAd() : IndicatorImp("AD", 1) {}

IAd::IAd(const KData& k) : IndicatorImp("AD", 1) {
    setParam<KData>("kdata", k);
    IAd::_calculate(Indicator());
}

IAd::~IAd() {}

void IAd::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    m_discard = 0;
    KData k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    value_t ad = 0.0;
    auto* dst = this->data();
    dst[m_discard] = 0.0;
    for (size_t i = m_discard + 1; i < total; i++) {
        const KRecord& r = k[i];
        value_t tmp = r.highPrice - r.lowPrice;
        if (tmp != 0.0) {
            // 多空对比 = [（收盘价- 最低价） - （最高价 - 收盘价）] / （最高价 - 最低价）
            ad += ((r.closePrice + r.closePrice - r.highPrice - r.lowPrice) / tmp) * r.transAmount;
        }
        dst[i] = ad;
    }
}

Indicator HKU_API AD() {
    return make_shared<IAd>()->calculate();
}

Indicator HKU_API AD(const KData& k) {
    return Indicator(make_shared<IAd>(k));
}

} /* namespace hku */
