/*
 * IAlign.cpp
 * 
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-20
 *      Author: fasiondog
 */

#include "IAlign.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAlign)
#endif


namespace hku {

IAlign::IAlign() : IndicatorImp("ALIGN", 1) {
    setParam<DatetimeList>("ref", DatetimeList());
}

IAlign::~IAlign() {

}

bool IAlign::check() {
    return true;
}

void IAlign::_calculate(const Indicator& ind) {
    DatetimeList dates = getParam<DatetimeList>("ref");
    if (dates.empty()) {
        dates = getContext().getDatetimeList();
    }
    size_t total = dates.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    size_t ind_total = ind.size();
    size_t ind_idx = 0;
    size_t i = 0;
    while (i < total) {
        if (dates[i] >= ind.getDatetime(ind_idx)) {
            _set(ind[ind_idx], i);
            i++;
        } else {
            _set(0.0, i);
            size_t j = i + 1;
            while (j < ind_total && ind.getDatetime(j) <= dates[i]) {
                _set(0.0, j);
                j++;
            }
            
            i = j;
            if (j == ind_total) {
                for(; i < total; i++) {
                    _set(0.0, i);
                }
                break;
            }
        }
    }
}


Indicator HKU_API ALIGN() {
    return Indicator(make_shared<IAlign>());
}

Indicator HKU_API ALIGN(const DatetimeList& ref) {
    IndicatorImpPtr p = make_shared<IAlign>();
    p->setParam<DatetimeList>("ref", ref);
    return Indicator(p);
}


} /* namespace hku */
