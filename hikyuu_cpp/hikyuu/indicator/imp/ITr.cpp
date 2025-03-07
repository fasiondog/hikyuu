/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-05
 *      Author: fasiondog
 */

#include "ITr.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ITr)
#endif

namespace hku {

ITr::ITr() : IndicatorImp("TR", 1) {}

ITr::~ITr() {}

ITr::ITr(const KData& k) : IndicatorImp("TR", 1) {
    setParam<KData>("kdata", k);
    ITr::_calculate(Indicator());
}

void ITr::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData kdata = getContext();
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);

    m_discard = 1;

    auto* k = kdata.data();
    auto* dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        value_t v1 = k[i].highPrice - k[i].lowPrice;
        value_t v2 = std::abs(k[i].highPrice - k[i - 1].closePrice);
        value_t v3 = std::abs(k[i].lowPrice - k[i - 1].closePrice);
        dst[i] = std::max(std::max(v1, v2), v3);
    }
}

Indicator HKU_API TR() {
    return make_shared<ITr>()->calculate();
}

Indicator HKU_API TR(const KData& k) {
    return Indicator(make_shared<ITr>(k));
}

} /* namespace hku */
