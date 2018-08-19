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

bool Vigor::check() {
    int n = getParam<int>("n");
    if (n < 1) {
        HKU_ERROR("Invalide param[n] must >= 1 ! [Vigor::Vigor]");
        return false;
    }

    return true;
}

void Vigor::_calculate(const Indicator& ind) {
    if (ind.getResultNumber() < 6) {
        HKU_ERROR("ind's result_num must > 6! [Vigor::_calculate]");
        return;
    }

    size_t total = ind.size();
    if (0 == total) {
        return;
    }

    int n = getParam<int>("n");

    m_discard = 1 + ind.discard();
    if (0 == total) {
        return;
    }

    PriceList tmp(total, Null<price_t>());
    for (size_t i = m_discard; i < total; ++i) {
        //tmp[i] = (kdata[i].closePrice - kdata[i-1].closePrice) * kdata[i].transCount;
        tmp[i] = (ind.get(i,3) - ind.get(i-1,3)) * ind.get(i,5);
    }

    Indicator ema = EMA(PRICELIST(tmp, m_discard), n);
    for (size_t i = 0; i < total; ++i) {
        _set(ema[i], i);
    }
}


Indicator HKU_API VIGOR(const KData& data, int n) {
    return Indicator(IndicatorImpPtr(new Vigor(data, n)));
}

Indicator HKU_API VIGOR(int n) {
    IndicatorImpPtr p = make_shared<Vigor>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API VIGOR(const Indicator& indicator, int n){
    IndicatorImpPtr p = make_shared<Vigor>();
    p->setParam<int>("n", n);
    p->calculate(indicator);
    return Indicator(p);
}

} /* namespace hku */
