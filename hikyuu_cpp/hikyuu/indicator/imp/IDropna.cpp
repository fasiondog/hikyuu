/*
 * IDropna.cpp
 * 
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-28
 *      Author: fasiondog
 */

#include "IDropna.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IDropna)
#endif


namespace hku {

IDropna::IDropna() : IndicatorImp("DROPNA", 1) {
    setParam<DatetimeList>("align_date_list", DatetimeList());
}

IDropna::~IDropna() {

}

bool IDropna::check() {
    return true;
}

void IDropna::_calculate(const Indicator& ind) {
    // ref_date_list 参数会影响 IndicatorImp 全局，勿随意修改
    size_t total = ind.size();
    if (ind.discard() >= total) {
        m_discard = 0;
        setParam<DatetimeList>("align_date_list", DatetimeList());
        return;
    }

    m_result_num = ind.getResultNumber();
    size_t row_len = total - ind.discard();
    price_t *buf = new price_t[m_result_num * row_len];
    if (!buf) {
        HKU_ERROR("Memory allocation failed! [IDropna::_calculate]");
        return;
    }

    DatetimeList dates;
    size_t pos = 0;
    for (size_t i = ind.discard(); i < total; i++) {
        bool had_nan = false;
        for (size_t r = 0; r < m_result_num; r++) {
            if (std::isnan(ind.get(i, r))) {
                had_nan = true;
                break;
            }
        }

        if (!had_nan) {
            dates.push_back(ind.getDatetime(i));
            for (size_t r = 0; r < m_result_num; r++) {
                buf[pos + r * m_result_num] = ind.get(i, r);
            }
            pos++;
        }
    }

    _readyBuffer(pos / m_result_num, m_result_num);

    for (size_t r = 0; r < m_result_num; r++) {
        int start = r * m_result_num;
        for (size_t i = 0; i < pos; i++) {
            _set(buf[start + i], i, r);
        }
    }

    delete[] buf;

    m_discard = 0;
    setParam<DatetimeList>("align_date_list", dates);
}


Indicator HKU_API DROPNA() {
    return Indicator(make_shared<IDropna>());
}


} /* namespace hku */
