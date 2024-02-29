/*
 * IDecline.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-3
 *      Author: fasiondog
 */

#include "IDecline.h"
#include "../Indicator.h"
#include "../crt/KDATA.h"
#include "../crt/REF.h"
#include "../crt/ALIGN.h"
#include "../../StockManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IDecline)
#endif

namespace hku {

IDecline::IDecline() : IndicatorImp("DECLINE", 1) {
    setParam<KQuery>("query", KQueryByIndex(-100));
    setParam<string>("market", "SH");
    setParam<int>("stk_type", STOCKTYPE_A);
    setParam<bool>("ignore_context", false);
}

IDecline::~IDecline() {}

bool IDecline::check() {
    return true;
}

void IDecline::_calculate(const Indicator& ind) {
    // ref_date_list 参数会影响 IndicatorImp 全局，勿随意修改
    string market;
    KQuery q;
    int stk_type = STOCKTYPE_A;
    bool ignore_context = getParam<bool>("ignore_context");
    KData k = getContext();
    if (!ignore_context && !k.empty()) {
        q = k.getQuery();
        Stock stk = k.getStock();
        market = stk.market();
        stk_type = stk.type();
    } else {
        market = getParam<string>("market");
        q = getParam<KQuery>("query");
        stk_type = getParam<int>("stk_type");
    }

    StockManager& sm = StockManager::instance();
    DatetimeList dates = sm.getTradingCalendar(q, market);
    size_t total = dates.size();
    if (total == 0) {
        m_discard = 0;
        _readyBuffer(0, 1);
        return;
    }

    m_discard = 1;
    _readyBuffer(total, 1);
    auto* dst = this->data();
    Indicator x = ALIGN(CLOSE() < REF(CLOSE(), 1), dates);
    for (auto iter = sm.begin(); iter != sm.end(); ++iter) {
        if ((stk_type <= STOCKTYPE_TMP && iter->type() != stk_type) ||
            (market != "" && iter->market() != market)) {
            continue;
        }
        x.setContext(*iter, q);
        auto const* xdata = x.data();
        for (size_t i = x.discard(); i < total; i++) {
            if (x.getDatetime(i) > iter->lastDatetime()) {
                break;
            }

            if (xdata[i]) {
                dst[i] = std::isnan(dst[i]) ? 1 : dst[i] + 1;
            }
        }
    }
}

Indicator HKU_API DECLINE(const KQuery& query, const string& market, int stk_type,
                          bool ignore_context) {
    IndicatorImpPtr p = make_shared<IDecline>();
    p->setParam<KQuery>("query", query);
    p->setParam<string>("market", market);
    p->setParam<int>("stk_type", stk_type);
    p->setParam<bool>("ignore_context", ignore_context);
    p->calculate();
    return Indicator(p);
}

} /* namespace hku */
