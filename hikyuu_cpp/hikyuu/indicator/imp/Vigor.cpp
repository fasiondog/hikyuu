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

Vigor::Vigor(): IndicatorImp("VIGOR", 1) {
    setParam<int>("n", 2);
}

Vigor::Vigor(int n): IndicatorImp() {
    setParam<int>("n", n);
}


Vigor::~Vigor() {

}

bool Vigor::check() {
    int n = getParam<int>("n");
    if (n < 1) {
        HKU_ERROR("Invalide param[n] must >= 1 ! [Vigor::Vigor]");
        return false;
    }

    return true;
}

void Vigor::_calculate(const Indicator& ind) {
    KData kdata = getCurrentKData();
    size_t total = kdata.size();
    _readyBuffer(total, 1);

    int n = getParam<int>("n");

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


Indicator HKU_API VIGOR(int n) {
    return make_shared<Vigor>(n)->calculate();
}

Indicator HKU_API VIGOR(const KData& data, int n) {
    Indicator v = VIGOR(n);
    v.setContext(data.getStock(), data.getQuery());
    return v;
}

Indicator HKU_API VIGOR(const Indicator& ind, int n) {
    return VIGOR(ind.getCurrentKData(), n);
}


} /* namespace hku */
