/*
 * IPriceList.cpp
 *
 *  Created on: 2013-2-12
 *      Author: fasiondog
 */

#include "IPriceList.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IPriceList)
#endif

namespace hku {

IPriceList::IPriceList() : IndicatorImp("PRICELIST", 1) {
    setParam<int>("result_index", 0);
    setParam<PriceList>("data", PriceList());
    setParam<int>("discard", 0);
}

IPriceList::IPriceList(const PriceList& data, int in_discard) : IndicatorImp("PRICELIST", 1) {
    setParam<int>("result_index", 0);
    setParam<PriceList>("data", data);
    setParam<int>("discard", in_discard);
}

IPriceList::~IPriceList() {}

void IPriceList::_checkParam(const string& name) const {
    if ("discard" == name) {
        HKU_ASSERT(getParam<int>("discard") >= 0);
    } else if ("result_index" == name) {
        HKU_ASSERT(getParam<int>("result_index") >= 0);
    }
}

void IPriceList::_calculate(const Indicator& data) {
    // 如果在叶子节点，直接取自身的data参数
    if (isLeaf()) {
        PriceList x = getParam<PriceList>("data");
        int discard = getParam<int>("discard");

        size_t total = x.size();
        _readyBuffer(total, 1);

        // 更新抛弃数量
        m_discard = discard > total ? total : discard;

        auto* dst = this->data();
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = x[i];
        }
        return;
    }

    // 不在叶子节点上，则忽略本身的data参数，认为其输入实际为函数入参中的data
    int result_index = getParam<int>("result_index");
    HKU_ERROR_IF_RETURN(result_index < 0 || result_index >= data.getResultNumber(), void(),
                        "result_index out of range!");

    size_t total = data.size();
    _readyBuffer(total, 1);

    auto const* src = data.data(result_index);
    auto* dst = this->data();
    for (size_t i = data.discard(); i < total; ++i) {
        dst[i] = src[i];
    }

    // 更新抛弃数量
    m_discard = data.discard();

    DatetimeList dates = data.getDatetimeList();
    if (!dates.empty()) {
        setParam<DatetimeList>("align_date_list", dates);
    }
}

Indicator HKU_API PRICELIST(const PriceList& data, int discard) {
    return make_shared<IPriceList>(data, discard)->calculate();
}

Indicator HKU_API PRICELIST(const PriceList& data, const DatetimeList& ds, int discard) {
    auto ret = PRICELIST(data, discard);
    HKU_CHECK(data.size() == ds.size(),
              "The data length must be the same as the length of the reference date list");
    ret.setParam<DatetimeList>("align_date_list", ds);
    return ret;
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

} /* namespace hku */
