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
    size_t total = dates.size();
    if (0 == total) {
        m_discard = 0;
        return;
    }

    size_t ind_idx = 0;
    for (size_t i = 0; i < total; i++) {
    }
}


Indicator HKU_API ALIGN() {
    return Indicator(make_shared<IAlign>());
}


} /* namespace hku */
