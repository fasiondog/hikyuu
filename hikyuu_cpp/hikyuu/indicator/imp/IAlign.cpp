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
    setParam<DatetimeList>("align_date_list", DatetimeList());
}

IAlign::~IAlign() {}

bool IAlign::check() {
    return true;
}

void IAlign::_calculate(const Indicator& ind) {
    // ref_date_list 参数会影响 IndicatorImp 全局，勿随意修改
    DatetimeList dates = getParam<DatetimeList>("align_date_list");
    size_t total = dates.size();
    m_result_num = ind.getResultNumber();
    _readyBuffer(total, m_result_num);

    size_t ind_total = ind.size();
    if (total == 0 || ind_total == 0) {
        m_discard = total;
        return;
    }

    // 处理本身没有上下文日期的指标
    if (ind.getDatetimeList().size() == 0) {
        if (ind_total <= total) {
            size_t offset = total - ind_total;
            m_discard = offset + ind.discard();
            for (size_t r = 0; r < m_result_num; r++) {
                auto const* src = ind.data(r);
                auto* dst = this->data(r);
                for (size_t i = m_discard; i < total; i++) {
                    dst[i] = src[i - offset];
                }
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

    // 其它有上下文日期对应的指标数据
    size_t ind_idx = 0;
    for (size_t i = 0; i < total; i++) {
        if (ind_idx >= ind_total) {
            break;
        }

        Datetime ind_date = ind.getDatetime(ind_idx);
        if (ind_date == dates[i]) {
            for (size_t r = 0; r < m_result_num; r++) {
                _set(ind.get(ind_idx, r), i, r);
            }
            ind_idx++;

        } else if (ind_date < dates[i]) {
            size_t j = ind_idx + 1;
            while (j < ind_total && ind.getDatetime(j) < dates[i]) {
                j++;
            }

            if (j >= ind_total) {
                if (i >= 1) {
                    for (size_t r = 0; r < m_result_num; r++) {
                        price_t val = ind.get(j - 1, r);
                        for (; i < total; i++) {
                            _set(val, i, r);
                        }
                    }
                }
                break;
            }

            if (ind.getDatetime(j) == dates[i]) {
                for (size_t r = 0; r < m_result_num; r++) {
                    _set(ind.get(j, r), i, r);
                }
            }

            ind_idx = j + 1;
        }
    }

    m_discard = total;
    for (size_t i = 0; i < total; i++) {
        bool all_not_null = true;
        for (size_t r = 0; r < m_result_num; r++) {
            if (std::isnan(get(i, r))) {
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

Indicator HKU_API ALIGN(const DatetimeList& ref) {
    IndicatorImpPtr p = make_shared<IAlign>();
    p->setParam<DatetimeList>("align_date_list", ref);
    return Indicator(p);
}

} /* namespace hku */
