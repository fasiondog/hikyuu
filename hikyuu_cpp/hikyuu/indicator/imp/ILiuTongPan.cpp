/*
 * ILiuTongPang.cpp
 *
 *  Created on: 2019年3月6日
 *      Author: fasiondog
 */

#include "ILiuTongPan.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ILiuTongPan)
#endif

namespace hku {

ILiuTongPan::ILiuTongPan() : IndicatorImp("LIUTONGPAN", 1) {}

ILiuTongPan::~ILiuTongPan() {}

ILiuTongPan::ILiuTongPan(const KData& k) : IndicatorImp("LIUTONGPAN", 1) {
    setParam<KData>("kdata", k);
    ILiuTongPan::_calculate(Indicator());
}

void ILiuTongPan::_calculate(const Indicator& data) {
    HKU_WARN_IF(!isLeaf() && !data.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    KData k = getContext();
    size_t total = k.size();
    if (total == 0) {
        return;
    }

    _readyBuffer(total, 1);

    Stock stock = k.getStock();
    StockWeightList sw_list = stock.getWeight();
    if (sw_list.size() == 0) {
        return;
    }

    auto* dst = this->data();
    size_t pos = 0;
    auto sw_iter = sw_list.begin();
    price_t pre_free_count = sw_iter->freeCount();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t free_count = sw_iter->freeCount();
        if (free_count == 0) {
            continue;  // 忽略流通盘为0的权息
        }

        while (pos < total && k[pos].datetime < sw_iter->datetime()) {
            dst[pos] = pre_free_count;
            pos++;
        }

        pre_free_count = free_count;
        if (pos >= total) {
            break;
        }
    }

    for (; pos < total; pos++) {
        dst[pos] = pre_free_count;
    }
}

Indicator HKU_API LIUTONGPAN() {
    return make_shared<ILiuTongPan>()->calculate();
}

Indicator HKU_API LIUTONGPAN(const KData& k) {
    return Indicator(make_shared<ILiuTongPan>(k));
}

} /* namespace hku */
