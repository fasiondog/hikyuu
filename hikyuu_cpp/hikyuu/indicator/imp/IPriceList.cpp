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
    setParam<PriceList>("data", PriceList());
    setParam<int>("discard", 0);
}

IPriceList::IPriceList(const PriceList& data, int in_discard) {
    setParam<int>("result_index", 0);
    setParam<PriceList>("data", data);
    setParam<int>("discard", in_discard);
    /*
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
    */
}


IPriceList::~IPriceList() {

}

bool IPriceList::check() {
    if (getParam<int>("discard") < 0) {
        return false;
    }
    return true;
}

void IPriceList::_calculate(const Indicator& data) {

    //如果本身是叶子节点且不存在父节点，即单独的节点，直接取自身的data参数
    if (m_optype == IndicatorImp::LEAF && !m_parent) {
        PriceList x = getParam<PriceList>("data");
        int discard = getParam<int>("discard");

        size_t total = x.size();
        _readyBuffer(total, 1);

        //更新抛弃数量
        m_discard = discard > total ? total : discard;

        for (size_t i = m_discard; i < total; ++i) {
            _set(x[i], i);
        }

        return;
    }

    if (m_optype == IndicatorImp::LEAF && m_parent) {
        PriceList x = getParam<PriceList>("data");
        int discard = getParam<int>("discard");

        KData kdata = getCurrentKData();
    
        size_t total = std::min(x.size(), kdata.size());
        _readyBuffer(total, 1);

        if (kdata.size() > x.size()) {
            discard = discard + kdata.size() - x.size();
        } else {
            if (kdata.size() + discard <= x.size()) {
                discard = x.size() - kdata.size();
            } else {
                discard = 0;
            }
        }

        for (size_t i = discard; i < total; ++i) {
            _set(x[i], i);
        }

        //更新抛弃数量
        m_discard = discard;
        return;
    }

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
    return Indicator(make_shared<IPriceList>(data, discard));
}

Indicator HKU_API PRICELIST(const Indicator& data, int result_index) {
    IndicatorImpPtr p = make_shared<IPriceList>();
    p->setParam<int>("result_index", result_index);
    //p->calculate(data);
    return Indicator(p);
}

Indicator HKU_API PRICELIST(int result_index) {
    IndicatorImpPtr p = make_shared<IPriceList>();
    p->setParam<int>("result_index", result_index);
    return Indicator(p);
}


/*Indicator HKU_API PRICELIST(price_t *data, size_t total) {
    return Indicator(make_shared<IPriceList>(data, total));
}*/

} /* namespace hku */
