/*
 * Vigor.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "Vigor.h"
#include "../crt/EMA.h"

namespace hku {

Vigor::Vigor(): IndicatorImp() {

}

Vigor::Vigor(const KData& kdata, int n): IndicatorImp(1, 1) {
    addParam<int>("n", n);
    if (n < 1) {
        HKU_ERROR("Invalide param[n] must >= 1 ! [Vigor::Vigor]");
        return;
    }

    size_t total = kdata.size();
    if (0 == total) {
        return;
    }

    IndicatorImpPtr tmp(new IndicatorImp(1, 1));
    tmp->_append(Null<price_t>());
    for (size_t i = 1; i < total; ++i) {
        KRecord today = kdata[i];
        tmp->_append((today.closePrice - kdata[i-1].closePrice) * today.transCount);
    }

    Indicator ema = EMA(Indicator(tmp), n);
    for (size_t i = 0; i < total; ++i) {
        _append(ema[i]);
    }
}


Vigor::~Vigor() {

}


string Vigor::name() const {
    return "VIGOR";
}


Indicator HKU_API VIGOR(const KData& data, int n) {
    return Indicator(IndicatorImpPtr(new Vigor(data, n)));
}

} /* namespace hku */
