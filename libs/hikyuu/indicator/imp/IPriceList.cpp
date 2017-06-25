/*
 * IPriceList.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#include "IPriceList.h"

namespace hku {

IPriceList::IPriceList() : IndicatorImp("PRICELIST") {
    setParam<int>("result_index", 0);
}

IPriceList::IPriceList(const PriceList& data, int in_discard) {
    setParam<int>("result_index", 0);
    size_t discard = 0;
    if (in_discard > 0) {
        discard = in_discard;
    }

    size_t total = data.size();
    _readyBuffer(total, 1);

    for (size_t i = discard; i < total; ++i) {
        _set(data[i], i);
    }

    //更新抛弃数量
    m_discard = discard;
}


IPriceList::IPriceList(price_t *data, size_t size): IndicatorImp("PRICELIST") {
    setParam<int>("result_index", 0);
    if (!data) {
        HKU_WARN("data pointer is NULL! [IPriceList::IPriceList]");
        return;
    }

    if (size == 0) {
        return;
    }

    _readyBuffer(size, 1);

    price_t null_price = Null<price_t>();
    size_t discard = 0;
    for (size_t i = 0; i < size; ++i) {
        _set(data[i], i);
        if (data[i] == null_price) {
            discard = i + 1;
        }
    }

    m_discard = discard;
    for (size_t i = 0; i < m_discard; ++i) {
        _set(null_price, i);
    }
}


IPriceList::~IPriceList() {

}

bool IPriceList::check() {
    return true;
}

void IPriceList::_calculate(const Indicator& data) {
    int result_index = getParam<int>("result_index");
    if (result_index < 0 || result_index >= data.getResultNumber()) {
        HKU_ERROR("result_index out of range! [IPriceList::IPriceList]");
        return;
    }

    size_t total = data.size();
    _readyBuffer(total, 1);

    for (size_t i = data.discard(); i < total; ++i) {
        _set(data.get(i, result_index), i);
    }

    //更新抛弃数量
    m_discard = data.discard();
}


Indicator HKU_API PRICELIST(const PriceList& data, int discard) {
    return Indicator(IndicatorImpPtr(new IPriceList(data, discard)));
}

Indicator HKU_API PRICELIST(const Indicator& data, int result_index) {
    IndicatorImpPtr p = make_shared<IPriceList>();
    p->setParam<int>("result_index", result_index);
    p->calculate(data);
    return Indicator(p);
}

Indicator HKU_API PRICELIST(int result_index) {
    IndicatorImpPtr p = make_shared<IPriceList>();
    p->setParam<int>("result_index", result_index);
    return Indicator(p);
}


Indicator HKU_API PRICELIST(price_t *data, size_t total) {
    return Indicator(IndicatorImpPtr(new IPriceList(data, total)));
}

} /* namespace hku */
