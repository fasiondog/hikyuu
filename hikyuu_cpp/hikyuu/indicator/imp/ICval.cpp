/*
 * ConstantValue.cpp
 *
 *  Created on: 2017年6月25日
 *      Author: Administrator
 */

#include "ICval.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ICval)
#endif

namespace hku {

ICval::ICval() : IndicatorImp("CVAL", 1) {
    m_need_self_alike_compare = true;
    setParam<double>("value", 0.0);
    setParam<int>("discard", 0);
}

ICval::ICval(double value, size_t discard) : IndicatorImp("CVAL", 1) {
    m_need_self_alike_compare = true;
    setParam<double>("value", value);
    setParam<int>("discard", discard);
}

ICval::~ICval() {}

void ICval::_checkParam(const string& name) const {
    if ("discard" == name) {
        HKU_ASSERT(getParam<int>("discard") >= 0);
    }
}

bool ICval::selfAlike(const IndicatorImp& other) const noexcept {
    HKU_IF_RETURN(isLeaf() && other.isLeaf(), true);
    return m_right && m_right->alike(*other.getRightNode());
}

void ICval::_calculate(const Indicator& data) {
    double value = getParam<double>("value");
    int discard = getParam<int>("discard");

    size_t total = 0;
    if (isLeaf()) {
        // 叶子节点
        const KData& k = getContext();
        if (k.getStock().isNull()) {
            _readyBuffer(1, 1);
            if (discard < 1) {
                m_discard = 0;
                _set(value, 0, 0);
            } else {
                m_discard = 1;
            }
            return;
        }

        total = k.size();
        if (0 == total) {
            return;
        }

        _readyBuffer(total, 1);

    } else {
        // 非叶子节点
        total = data.size();
        discard = data.discard() > discard ? data.discard() : discard;
    }

    m_discard = discard > total ? total : discard;

    size_t ret_num = data.getResultNumber();
    if (ret_num == 0) {
        ret_num = 1;
    }
    _readyBuffer(total, ret_num);

    for (size_t r = 0; r < ret_num; ++r) {
        auto* dst = this->data(r);
        for (size_t i = m_discard; i < total; ++i) {
            dst[i] = value;
        }
    }
}

void ICval::_increment_calculate(const Indicator& data, size_t start_pos) {
    double value = getParam<double>("value");

    size_t total = 0;
    if (isLeaf()) {
        // 叶子节点
        const KData& k = getContext();
        total = k.size();
        if (0 == total) {
            return;
        }

    } else {
        // 非叶子节点
        total = data.size();
    }

    for (size_t r = 0; r < m_result_num; ++r) {
        auto* dst = this->data(r);
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = value;
        }
    }
}

Indicator HKU_API CVAL(double value, size_t discard) {
    return make_shared<ICval>(value, discard)->calculate();
}

Indicator HKU_API CVAL(const Indicator& ind, double value, int discard) {
    auto p = make_shared<ICval>(value, discard);
    if (ind.getContext() == Null<KData>()) {
        // 传入的ind没有上下文时，如果忽略的数据长度和输入数据长度一致，则直接作为叶子节点
        // 因为 ind 底层可能包含了其他CVAL，但CVAL没有上下文时，size为1，类似的还有 PRICELIST
        return ind.discard() == ind.size() ? Indicator(p) : Indicator(p)(ind);
    }

    p->setContext(ind.getContext());
    return Indicator(p);
}

} /* namespace hku */
