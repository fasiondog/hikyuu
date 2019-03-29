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
    setParam<KData>("kdata", k);
    _calculate(Indicator());
}

bool ILiuTongPan::check() {
    return true;
}

void ILiuTongPan::_calculate(const Indicator& data) {
    KData k = getCurrentKData();
    size_t total = k.size();
    if (total == 0) {
        return;
    }
    
    _readyBuffer(total, 1);

    Stock stock = k.getStock();
    StockWeightList sw_list = stock.getWeight();
    if (sw_list.size() == 0){
        return;
    }

    size_t pos = 0;
    auto sw_iter = sw_list.begin();
    price_t free_count = 0;
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        free_count = sw_iter->freeCount();
        if (free_count == 0) {
            continue; //忽略流通盘为0的权息
        }

        while (pos < total && k[pos].datetime <= sw_iter->datetime()) {
            _set(free_count, pos);
            pos++;
        }
    }

    for (; pos < total; pos++) {
        _set(free_count, pos);
    }
}

Indicator HKU_API LIUTONGPAN() {
    return make_shared<ILiuTongPan>()->calculate();
}

Indicator HKU_API LIUTONGPAN(const KData& k) {
    return Indicator(make_shared<ILiuTongPan>(k));
}


} /* namespace hku */
