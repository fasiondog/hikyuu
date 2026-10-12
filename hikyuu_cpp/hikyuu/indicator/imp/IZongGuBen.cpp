/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-18
 *      Author: fasiondog
 */

#include "IZongGuBen.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IZongGuBen)
#endif

namespace hku {

IZongGuBen::IZongGuBen() : IndicatorImp("ZONGGUBEN", 1) {
    m_need_context = true;
}

IZongGuBen::~IZongGuBen() {}

void IZongGuBen::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);
    _increment_calculate(data, 0);
    updateDiscard(true);
}

void IZongGuBen::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& k = getContext();
    Stock stock = k.getStock();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0 || start_pos >= total, void());

    StockWeightList sw_list = stock.getWeight();
    HKU_IF_RETURN(sw_list.empty(), void());

    // Find the first ex-rights/ex-dividend record whose total shares are not 0
    price_t pre_total_count = 0.0;
    Datetime pre_sw_date;
    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        if (sw_iter->totalCount() > 0) {
            pre_total_count = sw_iter->totalCount();
            pre_sw_date = sw_iter->datetime();
            break;
        }
    }
    HKU_IF_RETURN(sw_iter == sw_list.end(), void());

    auto* dst = this->data();
    size_t pos = start_pos;
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t total_count = sw_iter->totalCount();
        Datetime cur_sw_date = sw_iter->datetime();
        if (total_count == 0) {
            continue;  // Ignore the ex-rights/ex-dividend record whose outstanding shares are 0
        }

        while (pos < total && k[pos].datetime < cur_sw_date) {
            if (k[pos].datetime >= pre_sw_date) {
                dst[pos] = pre_total_count;
            }
            pos++;
        }

        pre_total_count = total_count;
        pre_sw_date = cur_sw_date;
        if (pos >= total) {
            break;
        }
    }

    for (; pos < total; pos++) {
        if (k[pos].datetime >= pre_sw_date) {
            dst[pos] = pre_total_count;
        }
    }
}

Indicator HKU_API ZONGGUBEN() {
    return make_shared<IZongGuBen>()->calculate();
}

Indicator HKU_API ZONGGUBEN(const KData& k) {
    auto p = make_shared<IZongGuBen>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */
