/*
 * IVigor.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "IVigor.h"
#include "../crt/EMA.h"
#include "../crt/PRICELIST.h"
#include "../crt/KDATA.h"
#include "../crt/REF.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IVigor)
#endif

namespace hku {

IVigor::IVigor() : IndicatorImp("VIGOR", 1) {
    setParam<int>("n", 2);
}

IVigor::IVigor(int n) : IndicatorImp("VIGOR") {
    setParam<int>("n", n);
}

IVigor::~IVigor() {}

void IVigor::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 1);
    }
}

void IVigor::_calculate(const Indicator& ind) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData kdata = getContext();
    size_t total = kdata.size();
    _readyBuffer(total, 1);

    int n = getParam<int>("n");

    m_discard = 1;
    if (0 == total) {
        return;
    }

    auto const* ks = kdata.data();
    PriceList tmp(total, Null<price_t>());
    for (size_t i = 1; i < total; ++i) {
        tmp[i] = (ks[i].closePrice - ks[i - 1].closePrice) * ks[i].transCount;
    }

    Indicator ema = EMA(PRICELIST(tmp, 1), n);
    auto const* src = ema.data();
    auto* dst = this->data();
    for (size_t i = 0; i < total; ++i) {
        dst[i] = src[i];
    }
}

Indicator HKU_API VIGOR(int n) {
    return make_shared<IVigor>(n)->calculate();
}

Indicator HKU_API VIGOR(const KData& k, int n) {
    Indicator v = VIGOR(n);
    v.setContext(k);
    return v;
}

} /* namespace hku */
