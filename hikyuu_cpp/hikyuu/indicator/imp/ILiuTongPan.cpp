/*
 * ILiuTongPang.cpp
 *
 *  Created on: 2019年3月6日
 *      Author: fasiondog
 */

#include "ILiuTongPan.h"

namespace hku {

ILiuTongPan::ILiuTongPan() : IndicatorImp("LIUTONGPAN", 1) {

}

ILiuTongPan::~ILiuTongPan() {

}

ILiuTongPan::ILiuTongPan(const KData& k) : IndicatorImp("LIUTONGPAN", 1) {
    size_t total = k.size();
    _readyBuffer(total, 1);

    if (total == 0) {
        return;
    }

    Stock stock = k.getStock();
    StockWeightList sw_list = stock.getWeight();
    if (sw_list.size() == 0){
        return;
    }

    size_t pos = 0;
    auto sw_iter = sw_list.begin();
    price_t free_count = sw_iter++->freeCount();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        if (free_count == 0) {
            continue;
        }

        while (k[pos].datetime <= sw_iter->datetime()) {
            _set(free_count, pos);
            pos++;
        }

        free_count = sw_iter->freeCount();
    }

    for (; pos < total; pos++) {
        _set(free_count, pos);
    }
}

bool ILiuTongPan::check() {
    return true;
}

void ILiuTongPan::_calculate(const Indicator& data) {
    _readyBuffer(data.size(), 1);
}


Indicator HKU_API LIUTONGPAN(const KData& k) {
    return make_shared<ILiuTongPan>(k);
}


} /* namespace hku */
