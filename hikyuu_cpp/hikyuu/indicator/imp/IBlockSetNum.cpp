/*
 * IBlockSetNum.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2024-5-21
 *      Author: fasiondog
 */

#include "IBlockSetNum.h"
#include "../Indicator.h"
#include "../../StockManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBlockSetNum)
#endif

namespace hku {

IBlockSetNum::IBlockSetNum() : IndicatorImp("BLOCKSETNUM", 1) {
    setParam<KQuery>("query", KQueryByIndex(-100));
    setParam<Block>("block", Block());
    setParam<string>("market", "SH");
    setParam<bool>("ignore_context", false);
}

IBlockSetNum::~IBlockSetNum() {}

void IBlockSetNum::_checkParam(const string& name) const {
    if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    }
}

void IBlockSetNum::_calculate(const Indicator& ind) {
    Block block = getParam<Block>("block");
    bool ignore_context = getParam<bool>("ignore_context");
    KData k = getContext();
    DatetimeList dates;
    if (!ignore_context && !k.empty()) {
        dates = k.getDatetimeList();
    } else {
        KQuery q = getParam<KQuery>("query");
        dates = StockManager::instance().getTradingCalendar(q, getParam<string>("market"));
    }

    size_t total = dates.size();
    m_discard = 0;
    _readyBuffer(total, 1);
    HKU_IF_RETURN(total == 0, void());

    value_t zero = 0.0;
    auto* dst = this->data();
    for (size_t i = 0; i < total; i++) {
        dst[i] = zero;
    }

    for (auto iter = block.begin(); iter != block.end(); ++iter) {
        const Datetime& start_date = iter->startDatetime();
        Datetime last_date = iter->lastDatetime().isNull() ? Datetime::max() : iter->lastDatetime();
        for (size_t i = 0; i < total; i++) {
            if (dates[i] >= start_date && dates[i] <= last_date) {
                dst[i]++;
            }
        }
    }
}

Indicator HKU_API BLOCKSETNUM(const Block& block, const KQuery& query) {
    IndicatorImpPtr p = make_shared<IBlockSetNum>();
    p->setParam<KQuery>("query", query);
    p->setParam<Block>("block", block);
    if (query == Null<KQuery>()) {
        p->setParam<bool>("ignore_context", true);
    } else {
        p->calculate();
    }
    return Indicator(p);
}

Indicator HKU_API BLOCKSETNUM(const string& category, const string& name, const KQuery& query) {
    Block block = StockManager::instance().getBlock(category, name);
    return BLOCKSETNUM(block, query);
}

} /* namespace hku */
