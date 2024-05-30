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

IDropna::~IDropna() {}

void IDropna::_calculate(const Indicator& ind) {
    // ref_date_list 参数会影响 IndicatorImp 全局，勿随意修改
    size_t total = ind.size();
    if (ind.discard() >= total) {
        m_discard = 0;
        setParam<DatetimeList>("align_date_list", DatetimeList());
        _readyBuffer(0, ind.getResultNumber());
        return;
    }

    m_result_num = ind.getResultNumber();
    size_t row_len = total - ind.discard();

#if CPP_STANDARD >= CPP_STANDARD_17
    std::unique_ptr<price_t[]> buf = std::make_unique<price_t[]>(m_result_num * row_len);
#else
    std::unique_ptr<price_t[]> buf(new price_t[m_result_num * row_len]);
#endif

    if (!buf) {
        HKU_ERROR("Memory allocation failed!");
        return;
    }

    DatetimeList dates;
    size_t pos = 0;
    for (size_t i = ind.discard(); i < total; i++) {
        bool has_nan = false;
        for (size_t r = 0; r < m_result_num; r++) {
            if (std::isnan(ind.get(i, r))) {
                has_nan = true;
                break;
            }
        }

        if (!has_nan) {
            dates.push_back(ind.getDatetime(i));
            for (size_t r = 0; r < m_result_num; r++) {
                buf[pos + r * m_result_num] = ind.get(i, r);
            }
            pos++;
        }
    }

    _readyBuffer(pos / m_result_num, m_result_num);

    for (size_t r = 0; r < m_result_num; r++) {
        auto* dst = this->data(r);
        int start = r * m_result_num;
        for (size_t i = 0; i < pos; i++) {
            dst[i] = buf[start + i];
        }
    }

    m_discard = 0;
    setParam<DatetimeList>("align_date_list", dates);
}

Indicator HKU_API DROPNA() {
    return Indicator(make_shared<IDropna>());
}

} /* namespace hku */
