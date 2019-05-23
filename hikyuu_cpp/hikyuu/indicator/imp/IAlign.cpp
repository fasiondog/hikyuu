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
    setParam<DatetimeList>("ref_date_list", DatetimeList());
}

IAlign::~IAlign() {

}

bool IAlign::check() {
    return true;
}

void IAlign::_calculate(const Indicator& ind) {
    DatetimeList dates = getParam<DatetimeList>("ref_date_list");
    if (dates.empty()) {
        dates = getContext().getDatetimeList();
    }

    size_t total = dates.size();
    m_result_num = ind.getResultNumber();
    _readyBuffer(total, m_result_num);

    size_t ind_total = ind.size();
    if (total == 0 || ind_total == 0) {
        m_discard = 0;
        return;
    }

    size_t ind_idx = 0;
    for (size_t i = 0; i < total; i++) {
        Datetime ind_date = ind.getDatetime(ind_idx);
        if (ind_date == dates[i]) {
            for (size_t r = 0; r < m_result_num; r++) {
                _set(ind.get(ind_idx, r), i, r);
            }
            ind_idx++;
        
        } else if (ind_date < dates[i]) {
            size_t j = i + 1;
            while (j < ind_total && ind.getDatetime(j) < dates[i]) {
                j++;
            }

            if (j == ind_total) {
                if (i >= 1) {
                    for(; i < total; i++) {
                        size_t pos = i - 1;
                        for (size_t r = 0; r < m_result_num; r++) {
                            _set(get(pos, r), i, r);
                        }
                    }
                }
                break;
            }

            if (ind.getDatetime(j) == dates[i]) {
                for (size_t r = 0; r < m_result_num; r++) {
                    _set(ind.get(j, r), i, r);
                }
            } else {
                if (i >= 1) {
                    for (size_t r = 0; r < m_result_num; r++) {
                        _set(get(i-1, r), i, r);
                    }
                }
            }

            ind_idx = j + 1;

        } else { //ind_date > dates[i]

        }
    }

    bool all_not_null = false;
    m_discard = total;
    for (size_t i = 0; i < total; i++) {
        all_not_null = true;
        for (size_t r = 0; r < m_result_num; r++) {
            if (get(i, r) == Null<price_t>()) {
                all_not_null = false;
                break;
            }
        }

        if (all_not_null) {
            m_discard = i;
            break;
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
