/*
 * Vigor.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "Vigor.h"
#include "../crt/EMA.h"
#include "../crt/PRICELIST.h"
#include "../crt/KDATA.h"
#include "../crt/REF.h"

namespace hku {

Vigor::Vigor(): IndicatorImp("VIGOR") {
    setParam<int>("n", 2);
}

Vigor::Vigor(const KData& kdata, int n): IndicatorImp() {
    size_t total = kdata.size();
    _readyBuffer(total, 1);

    setParam<int>("n", n);
    if (n < 1) {
        HKU_ERROR("Invalide param[n] must >= 1 ! [Vigor::Vigor]");
        return;
    }

    m_discard = 1;
    if (0 == total) {
        return;
    }

    PriceList tmp(total, Null<price_t>());
    for (size_t i = 1; i < total; ++i) {
        tmp[i] = (kdata[i].closePrice - kdata[i-1].closePrice) * kdata[i].transCount;
    }

    Indicator ema = EMA(PRICELIST(tmp, 1), n);
    for (size_t i = 0; i < total; ++i) {
        _set(ema[i], i);
    }
}


Vigor::~Vigor() {

}


Indicator HKU_API VIGOR(const KData& data, int n) {
    return Indicator(IndicatorImpPtr(new Vigor(data, n)));
}

} /* namespace hku */
