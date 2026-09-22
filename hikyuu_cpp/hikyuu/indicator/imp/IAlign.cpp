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

IAlign::IAlign() : IndicatorImp("ALIGN") {
    setParam<DatetimeList>("align_date_list", DatetimeList());  // The date sequence to align to
    setParam<bool>("fill_null", true);  // Whether to fill the missing data with nan
}

IAlign::~IAlign() {}

void IAlign::_calculate(const Indicator& ind) {
    // The ref_date_list parameter affects the IndicatorImp globally, do not modify it at will
    DatetimeList dates = getParam<DatetimeList>("align_date_list");
    size_t total = dates.size();

    // If align_date_list is invalid, try to use the dates in its own context as the reference dates
    if (0 == total) {
        dates = getContext().getDatetimeList();
        total = dates.size();
    }

    m_result_num = ind.getResultNumber();
    _readyBuffer(total, m_result_num);

    size_t ind_total = ind.size();
    if (total == 0 || ind_total == 0) {
        m_discard = total;
        return;
    }

    bool fill_null = getParam<bool>("fill_null");

    // Handle the case where the passed indicator itself has no context dates and cannot be
    // aligned:
    // 1. ignore the fill_null parameter;
    // 2. if the data length is not greater than the date sequence length, align at the right end,
    //    i.e. the last data corresponds to the last date and the missing data in front is
    //    discarded;
    // 3. if the data length is greater than the date sequence length, align at the right end and
    //    discard the data in front that exceeds the date sequence.
    DatetimeList ind_dates = ind.getDatetimeList();
    if (ind_dates.size() == 0) {
        if (ind_total <= total) {
            size_t offset = total - ind_total;
            m_discard = offset + ind.discard();
            for (size_t r = 0; r < m_result_num; r++) {
                auto const* src = ind.data(r);
                auto* dst = this->data(r);
                memcpy(dst + m_discard, src + ind.discard(),
                       sizeof(IndicatorImp::value_t) * (total - m_discard));
            }
            return;

        } else {
            // ind_total > total
            m_discard = 0;
            size_t offset = ind_total - total;
            if (ind.discard() > offset) {
                m_discard = ind.discard() - offset;
            }

            for (size_t r = 0; r < m_result_num; r++) {
                auto const* src = ind.data(r);
                auto* dst = this->data(r);
                for (size_t i = m_discard; i < total; i++) {
                    dst[i] = src[i + offset];
                }
            }
            return;
        }
    }

    // The other indicator data that has the corresponding context dates
    // 1. If there is no exactly equal date, take the data of the closest date earlier than the
    //    corresponding date;
    // 2. if there is a corresponding date, take the data of that date.
    if (fill_null) {
        size_t ind_idx = ind.discard();
        for (size_t i = 0; i < total; i++) {
            if (ind_idx >= ind_total) {
                break;
            }

            const Datetime& ind_date = ind_dates[ind_idx];
            if (ind_date == dates[i]) {
                for (size_t r = 0; r < m_result_num; r++) {
                    _set(ind.get(ind_idx, r), i, r);
                }
                ind_idx++;

            } else if (ind_date < dates[i]) {
                size_t j = ind_idx + 1;
                while (j < ind_total && ind_dates[j] < dates[i]) {
                    j++;
                }

                if (j >= ind_total) {
                    break;
                }

                if (ind_dates[j] == dates[i]) {
                    for (size_t r = 0; r < m_result_num; r++) {
                        _set(ind.get(j, r), i, r);
                    }
                }

                ind_idx = j + 1;
            }
        }

    } else {
        if (ind_dates[0] > dates[total - 1]) {
            // If the first data date > the last reference date, ignore everything
            m_discard = total;
            return;

        } else if (dates[0] > ind_dates[ind_total - 1]) {
            // If all the reference dates are later than the last date of ind_dates, use the last
            // data of ind_dates directly
            for (size_t r = 0; r < m_result_num; r++) {
                value_t val = ind.get(ind_total - 1, r);
                auto* dst = this->data(r);
                for (size_t i = 0; i < total; i++) {
                    dst[i] = val;
                }
            }

        } else {
            size_t pos = 0;
            for (size_t i = 0; i < total; i++) {
                if (dates[i] >= ind_dates[0]) {
                    pos = i;
                    break;
                }
            }
            for (size_t ind_idx = 0; ind_idx < ind_total; ind_idx++) {
                const Datetime& ind_date = ind_dates[ind_idx];
                for (size_t i = pos; i < total; i++) {
                    if (dates[i] < ind_date) {
                        for (size_t r = 0; r < m_result_num; r++) {
                            _set(ind.get(ind_idx - 1, r), i, r);
                        }
                    } else if (dates[i] == ind_date) {
                        for (size_t r = 0; r < m_result_num; r++) {
                            _set(ind.get(ind_idx, r), i, r);
                        }
                    } else {
                        pos = i;
                        break;
                    }
                }
                if (dates[pos] >= ind_dates[ind_total - 1]) {
                    break;
                }
            }
            if (pos < total) {
                if (dates[pos] >= ind_dates[ind_total - 1]) {
                    for (size_t r = 0; r < m_result_num; r++) {
                        for (size_t i = pos; i < total; i++) {
                            _set(ind.get(ind_total - 1, r), i, r);
                        }
                    }
                } else {
                    for (size_t r = 0; r < m_result_num; r++) {
                        auto* dst = this->data(r);
                        for (size_t i = pos; i < total; i++) {
                            dst[i] = dst[i - 1];
                        }
                    }
                }
            }
        }
    }

    // Force updating m_discard again
    m_discard = 0;
    updateDiscard();
}

Indicator HKU_API ALIGN(bool fill_null) {
    IndicatorImpPtr p = make_shared<IAlign>();
    p->setParam<bool>("fill_null", fill_null);
    return Indicator(p);
}

Indicator HKU_API ALIGN(const DatetimeList& ref, bool fill_null) {
    IndicatorImpPtr p = make_shared<IAlign>();
    p->setParam<DatetimeList>("align_date_list", ref);
    p->setParam<bool>("fill_null", fill_null);
    return Indicator(p);
}

Indicator HKU_API ALIGN(DatetimeList&& ref, bool fill_null) {
    IndicatorImpPtr p = make_shared<IAlign>();
    p->setParam<DatetimeList>("align_date_list", std::move(ref));
    p->setParam<bool>("fill_null", fill_null);
    return Indicator(p);
}

} /* namespace hku */
