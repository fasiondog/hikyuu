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
    if (getParam<int>("discard") < 0 || getParam<int>("result_index") < 0) {
        return false;
    }

    return true;
}

void IPriceList::_calculate(const Indicator& data) {

    //如果在叶子节点且不存在父节点，即单独的节点，直接取自身的data参数
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

    //如果在叶子节点且存在父节点，则取当前上下文中的KData，总长须等于kdata的长度
    if (m_optype == IndicatorImp::LEAF && m_parent) {
        PriceList x = getParam<PriceList>("data");
        int discard = getParam<int>("discard");

        KData kdata = getCurrentKData();
    
        size_t total = kdata.size();
        _readyBuffer(total, 1);

        if (discard >= total) {
            m_discard = total;
            return;
        }

        size_t x_len = x.size();
        if (total > x_len) {
            m_discard = discard + total - x_len;
        } else if (total + discard > x_len) {
            m_discard = discard + total - x_len;
        } else {
            m_discard = 0;
        }

        int diff = x_len - total;
        for (size_t i = m_discard; i < total; ++i) {
            _set(x[i+diff], i);
        }

        return;
    }

    //不在叶子节点上，则忽略本身的data参数，认为其输入实际为函数入参中的data
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
    return make_shared<IPriceList>(data, discard)->calculate();
}

Indicator HKU_API PRICELIST(const Indicator& data, int result_index) {
    IndicatorImpPtr p = make_shared<IPriceList>();
    p->setParam<int>("result_index", result_index);
    return Indicator(p)(data);
}

Indicator HKU_API PRICELIST(int result_index) {
    IndicatorImpPtr p = make_shared<IPriceList>();
    p->setParam<int>("result_index", result_index);
    return Indicator(p);
}


Indicator HKU_API PRICELIST(price_t *data, size_t total) {
    PriceList x(data, data+total);
    return PRICELIST(x, 0);
}

} /* namespace hku */
