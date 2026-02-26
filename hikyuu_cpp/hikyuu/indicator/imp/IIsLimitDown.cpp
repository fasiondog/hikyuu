/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

#include "IIsLimitDown.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIsLimitDown)
#endif

namespace hku {

IIsLimitDown::IIsLimitDown() : IndicatorImp("ISLIMITDOWN", 1) {
    m_need_context = true;
}

IIsLimitDown::~IIsLimitDown() {}

void IIsLimitDown::_calculate(const Indicator& ind) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!",
                getParam<string>("kpart"));

    size_t total = getContext().size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    m_discard = 1;  // 第一根K线没有前一根K线, 无法判断是否跌停, 直接舍弃
    _increment_calculate(ind, 0);
}

void IIsLimitDown::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& kdata = getContext();
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0, void());

    value_t limit_down = 0.0;
    const Stock& stock = kdata.getStock();
    if (stock.type() == STOCKTYPE_A) {
        limit_down = 0.9;  // A股跌停幅度为10%, 但ST股票跌停幅度为5%, 由于没有ST历史日期暂不处理
    } else if (stock.type() == STOCKTYPE_A_BJ) {
        limit_down = 0.7;  // 北交所跌停幅度为30%
    } else if (stock.type() == STOCKTYPE_GEM || stock.type() == STOCKTYPE_START) {
        limit_down = 0.8;  // 创业板和科创板跌停幅度为20%
    }

    auto const* ks = kdata.data();
    auto* dst = this->data();
    if (limit_down > 0.0) {
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = value_t(ks[i].closePrice <= roundEx(ks[i - 1].closePrice * limit_down));
        }
    } else {
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = 0.0;  // 不支持的股票类型, 默认为非跌停
        }
    }
}

Indicator HKU_API ISLIMITDOWN() {
    return make_shared<IIsLimitDown>()->calculate();
}

Indicator HKU_API ISLIMITDOWN(const KData& k) {
    auto p = make_shared<IIsLimitDown>();
    p->setContext(k);
    return Indicator(p);
}

}  // namespace hku