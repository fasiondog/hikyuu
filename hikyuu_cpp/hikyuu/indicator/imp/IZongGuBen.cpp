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
}

void IZongGuBen::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& k = getContext();
    Stock stock = k.getStock();
    size_t total = k.size();
    StockWeightList sw_list = stock.getWeight();
    if (sw_list.size() == 0) {
        return;
    }

    auto* dst = this->data();
    size_t pos = start_pos;
    auto sw_iter = sw_list.begin();
    price_t pre_total_count = sw_iter->totalCount();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t total_count = sw_iter->totalCount();
        if (total_count == 0) {
            continue;  // 忽略流通盘为0的权息
        }

        while (pos < total && k[pos].datetime < sw_iter->datetime()) {
            dst[pos] = pre_total_count;
            pos++;
        }

        pre_total_count = total_count;
        if (pos >= total) {
            break;
        }
    }

    for (; pos < total; pos++) {
        dst[pos] = pre_total_count;
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
