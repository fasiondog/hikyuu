/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-27
 *      Author: fasiondog
 */

#include "ISlice.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISlice)
#endif

namespace hku {

ISlice::ISlice() : IndicatorImp("SLICE", 1) {
    setParam<int>("result_index", 0);
    setParam<PriceList>("data", PriceList());
    setParam<int64_t>("start", 0);
    setParam<int64_t>("end", Null<int64_t>());
}

ISlice::ISlice(const PriceList& data, int64_t start, int64_t end) : IndicatorImp("SLICE", 1) {
    setParam<int>("result_index", 0);
    setParam<PriceList>("data", data);
    setParam<int64_t>("start", start);
    setParam<int64_t>("end", end);
}

ISlice::~ISlice() {}

void ISlice::_checkParam(const string& name) const {
    if ("result_index" == name) {
        HKU_ASSERT(getParam<int>("result_index") >= 0);
    }
}

void ISlice::_calculate(const Indicator& data) {
    // 如果在叶子节点，直接取自身的data参数
    if (isLeaf()) {
        m_discard = 0;
        PriceList x = getParam<PriceList>("data");
        size_t total = x.size();
        int64_t startix = getParam<int64_t>("start");
        if (startix < 0) {
            startix = total + startix;
        }
        HKU_ERROR_IF_RETURN(startix < 0 || size_t(startix) >= total, void(), "start {}, total {}",
                            startix, total);

        int64_t endix = getParam<int64_t>("end");
        if (endix == Null<int64_t>()) {
            endix = total;
        } else if (endix < 0) {
            endix = total + endix;
        }
        HKU_IF_RETURN(endix < 0 || size_t(endix) > total || startix == endix, void());

        _readyBuffer(endix - startix, 1);
        auto* dst = this->data();
        for (int64_t i = startix; i < endix; ++i) {
            dst[i - startix] = x[i];
        }
        return;
    }

    // 不在叶子节点上，则忽略本身的data参数，认为其输入实际为函数入参中的data
    int result_index = getParam<int>("result_index");
    HKU_ERROR_IF_RETURN(result_index < 0 || result_index >= data.getResultNumber(), void(),
                        "result_index out of range!");

    size_t total = data.size();
    int64_t startix = getParam<int64_t>("start");
    if (startix < 0) {
        startix = total + startix;
    }
    HKU_IF_RETURN(startix < 0 || size_t(startix) >= total, void());

    int64_t endix = getParam<int64_t>("end");
    if (endix == Null<int64_t>()) {
        endix = total;
    } else if (endix < 0) {
        endix = total + endix;
    }
    HKU_IF_RETURN(endix < 0 || size_t(endix) > total || startix == endix, void());

    _readyBuffer(endix - startix, 1);

    auto const* src = data.data(result_index);
    auto* dst = this->data();
    for (int64_t i = startix; i < endix; ++i) {
        dst[i - startix] = src[i];
    }

    // 更新抛弃数量
    m_discard = data.discard() <= size_t(startix) ? 0 : data.discard() - startix;
}

Indicator HKU_API SLICE(const PriceList& data, int64_t start, int64_t end) {
    return make_shared<ISlice>(data, start, end)->calculate();
}

Indicator HKU_API SLICE(int64_t start, int64_t end, int result_index) {
    IndicatorImpPtr p = make_shared<ISlice>();
    p->setParam<int>("result_index", result_index);
    p->setParam<int64_t>("start", start);
    p->setParam<int64_t>("end", end);
    return Indicator(p);
}

}  // namespace hku