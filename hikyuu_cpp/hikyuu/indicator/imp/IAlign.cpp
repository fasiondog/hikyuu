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

    m_result_num = ind.getResultNumber();
    _readyBuffer(total, m_result_num);

    size_t ind_total = ind.size();
    size_t ind_idx = 0;
    for (size_t i = 0; i < total; i++) {
        Datetime ind_date = ind.getDatetime(ind_idx);
        if (dates[i] == ind_date) {
            for (size_t r = 0; r < m_result_num; r++) {
                _set(ind[ind_idx], i, r);
            }
        
        } else if (dates[i] > ind_date) {
            size_t j = i + 1;
            while (j < ind_total && dates[i] >= ind.getDatetime(j)) {
                j++;
            }

            if (j == ind_total) {
                for(; i < total; i++) {
                    for (size_t r = 0; r < m_result_num; r++) {
                        _set(0.0, i, r);
                    }
                }
                break;
            }

            if (ind.getDatetime(j) == dates[i]) {
                for (size_t r = 0; r < m_result_num; r++) {
                    _set(ind[j], i, r);
                }
            }

            ind_idx = j;

        } else { //dates[i] < ind_date

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
