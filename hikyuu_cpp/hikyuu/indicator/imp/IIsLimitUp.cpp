/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-25
 *      Author: fasiondog
 */

#include "IIsLimitUp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIsLimitUp)
#endif

namespace hku {

IIsLimitUp::IIsLimitUp() : IndicatorImp("ISLIMITUP", 1) {
    m_need_context = true;
}

IIsLimitUp::~IIsLimitUp() {}

void IIsLimitUp::_calculate(const Indicator& ind) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!",
                getParam<string>("kpart"));

    size_t total = getContext().size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    m_discard = 1;  // 第一根K线没有前一根K线, 无法判断是否涨停, 直接舍弃
    _increment_calculate(ind, 0);
}

void IIsLimitUp::_increment_calculate(const Indicator& data, size_t start_pos) {
    const KData& kdata = getContext();
    size_t total = kdata.size();
    HKU_IF_RETURN(total == 0, void());

    value_t limit_up = 0.0;
    const Stock& stock = kdata.getStock();
    if (stock.type() == STOCKTYPE_A) {
        limit_up = 1.1;  // A股涨停幅度为10%, 但ST股票涨停幅度为5%, 由于没有ST历史日期暂不处理
    } else if (stock.type() == STOCKTYPE_A_BJ) {
        limit_up = 1.3;  // 北交所涨停幅度为30%
    } else if (stock.type() == STOCKTYPE_GEM || stock.type() == STOCKTYPE_START) {
        limit_up = 1.2;  // 创业板和科创板涨停幅度为20%
    }

    auto const* ks = kdata.data();
    auto* dst = this->data();
    if (limit_up > 0.0) {
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = value_t(ks[i].closePrice >= roundEx(ks[i - 1].closePrice * limit_up));
        }
    } else {
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = 0.0;  // 不支持的股票类型, 默认为非涨停
        }
    }
}

Indicator HKU_API ISLIMITUP() {
    return make_shared<IIsLimitUp>()->calculate();
}

Indicator HKU_API ISLIMITUP(const KData& k) {
    auto p = make_shared<IIsLimitUp>();
    p->setContext(k);
    return Indicator(p);
}

}  // namespace hku