/*
 * IPriceList.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#include "IPriceList.h"

namespace hku {

IPriceList::IPriceList() : IndicatorImp() {

}

IPriceList::IPriceList(const PriceList& price_list): IndicatorImp(0, 1) {
    size_t total = price_list.size();
    int discard = 0;
    price_t null_price = Null<price_t>();
    for (size_t i = 0; i < total; ++i) {
        _append(price_list[i]);
        if (price_list[i] == null_price) {
            discard = i + 1;
        }
    }
    m_discard = discard;
    for (int i = 0; i < m_discard; ++i) {
        _set(null_price, i);
    }
}


IPriceList::IPriceList(const PriceList& data, size_t start, size_t end)
: IndicatorImp(0, 1) {
    size_t total = data.size();
    int discard = start;
    price_t null_price = Null<price_t>();
    size_t endPos = end > total ? total : end;
    for (size_t i = start; i < endPos; ++i) {
        _append(data[i]);
        if (data[i] == null_price) {
            discard = i + 1;
        }
    }
    m_discard = discard  - start;
    for (int i = 0; i < m_discard; ++i) {
        _set(null_price, i);
    }
}


IPriceList::IPriceList(const Indicator& data, size_t result_num,
        size_t start, size_t end): IndicatorImp(0, 1) {
    if (result_num >= data.getResultNumber()) {
        HKU_ERROR("result_num out of range! [IPriceList::IPriceList]");
        return;
    }

    size_t total = data.size();
    size_t endPos = end > total ? total : end;
    for (size_t i = start; i < endPos; ++i) {
        _append(data.get(i, result_num));
    }

    //更新抛弃数量
    if (start < data.discard()) {
        m_discard = data.discard() - start;
    }
}


IPriceList::IPriceList(price_t *data, size_t size): IndicatorImp(0, 1) {
    if (data) {
        int discard = 0;
        price_t null_price = Null<price_t>();
        for (size_t i = 0; i < size; ++i) {
            _append(data[i]);
            if (data[i] == null_price) {
                discard = i + 1;
            }
            m_discard = discard;
            for (int i = 0; i < m_discard; ++i) {
                _set(null_price, i);
            }
        }
    } else {
        HKU_WARN("data pointer is NULL! [IPriceList::IPriceList]");
    }
}


IPriceList::~IPriceList() {

}


string IPriceList::name() const {
    return "PRICELIST";
}


Indicator HKU_API PRICELIST(const PriceList& price_list) {
    return Indicator(IndicatorImpPtr(new IPriceList(price_list)));
}

Indicator HKU_API PRICELIST(const PriceList& data, size_t start, size_t end) {
    return Indicator(IndicatorImpPtr(new IPriceList(data, start, end)));
}

Indicator HKU_API PRICELIST(const Indicator& data, size_t result_num,
        size_t start, size_t end) {
    return Indicator(IndicatorImpPtr(new IPriceList(data, result_num,
            start, end)));
}

Indicator HKU_API PRICELIST(price_t *data, size_t total) {
    return Indicator(IndicatorImpPtr(new IPriceList(data, total)));
}

} /* namespace hku */
