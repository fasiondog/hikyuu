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
    setParam<DatetimeList>("align_date_list", DatetimeList());  // 要对齐的日期序列（须已升序排列）
    setParam<bool>("fill_null", true);  // 缺失的数据是否使用 nan 填充
}

IAlign::~IAlign() {}

bool IAlign::check() {
    return true;
}

void IAlign::_calculate(const Indicator& ind) {
    // ref_date_list 参数会影响 IndicatorImp 全局，勿随意修改
    DatetimeList dates = getParam<DatetimeList>("align_date_list");
    size_t total = dates.size();

    // 如果 align_date_list 无效，则尝试取自身上下文中的日期作为参考日期
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

    // 处理传入的数据本身没有上下文日期的指标，无法对标的情况:
    // 1.如果 fill_null, 则直接返回，m_discard 标记全部
    // 2.数据长度小于等于日期序列长度，则按右对齐，即最后的数据对应最后的日期，前面缺失的数据做抛弃处理
    // 3.数据长度大于日期序列长度，按右对其，前面超出日期序列的数据丢弃
    DatetimeList ind_dates = ind.getDatetimeList();
    if (ind_dates.size() == 0) {
        if (fill_null) {
            m_discard = total;
            return;
        }

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

    // 其它有上下文日期对应的指标数据
    // 1. 如果没有刚好相等的日期，则取小于对应日期且最靠近对应日期的数据
    // 2. 如果有对应的日期，取对应日期的数据
    size_t ind_idx = ind.discard();
    for (size_t i = 0; i < total; i++) {
        if (ind_idx >= ind_total) {
            if (!fill_null) {
                size_t pos = ind_total - 1;
                for (size_t r = 0; r < m_result_num; r++) {
                    for (size_t j = i; j < total; j++) {
                        _set(ind.get(pos, r), j, r);
                    }
                }
            }
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
                if (!fill_null) {
                    for (size_t r = 0; r < m_result_num; r++) {
                        price_t val = ind.get(j - 1, r);
                        for (; i < total; i++) {
                            _set(val, i, r);
                        }
                    }
                }
                break;
            }

            if (ind_dates[j] == dates[i]) {
                for (size_t r = 0; r < m_result_num; r++) {
                    _set(ind.get(j, r), i, r);
                }
            } else if (!fill_null && j < ind_total) {
                for (size_t r = 0; r < m_result_num; r++) {
                    _set(ind.get(j - 1, r), i, r);
                }
            }

            ind_idx = j + 1;
        }
    }

    if (fill_null) {
        m_discard = total;
    }
    size_t i = 0;
    while (i < total) {
        size_t not_null_count = 0;
        for (size_t r = 0; r < m_result_num; r++) {
            if (!std::isnan(get(i, r))) {
                not_null_count++;
            }
        }
        if (not_null_count == m_result_num) {
            m_discard = i;
            break;
        }
        i++;
    }
    if (i == total) {
        m_discard = total;
    }
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

} /* namespace hku */
