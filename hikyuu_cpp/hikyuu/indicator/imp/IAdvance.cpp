/*
 * IAdvance.cpp
 * 
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-20
 *      Author: fasiondog
 */

#include "IAdvance.h"
#include "../Indicator.h"
#include "../crt/KDATA.h"
#include "../crt/REF.h"
#include "../crt/ALIGN.h"
#include "../../StockManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAdvance)
#endif


namespace hku {

IAdvance::IAdvance() : IndicatorImp("ADVANCE", 1) {
    setParam<KQuery>("query", KQueryByIndex(-100));
    setParam<string>("market", "SH");
    setParam<int>("stk_type", STOCKTYPE_A);
}

IAdvance::~IAdvance() {

}

bool IAdvance::check() {
    return true;
}

void IAdvance::_calculate(const Indicator& ind) {
    // ref_date_list 参数会影响 IndicatorImp 全局，勿随意修改
    string market = getParam<string>("market");
    KQuery q = getParam<KQuery>("query");
    int stk_type = getParam<int>("stk_type");
    StockManager& sm = StockManager::instance();

    DatetimeList dates = sm.getTradingCalendar(q, market);
    size_t total = dates.size();
    if (total == 0) {
        m_discard = 0;
        _readyBuffer(0, 1);
        return;
    }

    vector<size_t> stk_total(total, 0);
    vector<size_t> stk_ad(total, 0);
    Indicator x = ALIGN(IF(CLOSE() > REF(CLOSE(), 1), 1.0, 0.0), dates);
    for (auto iter = sm.begin(); iter != sm.end(); ++iter) {
        if (iter->market() != market || iter->type() != stk_type) {
            continue;
        }
        KData k = iter->getKData(q);
        x.setContext(k);
        for (size_t i = x.discard(); i < total; i++) {
            if (x.getDatetime(i) > iter->lastDatetime()) {
                break;
            }

            stk_total[i]++;

            if (x[i] == 1.0) {
                stk_ad[i]++;
            }
        }
    }

    m_discard = 1;
    _readyBuffer(total, 1);
    for (size_t i = m_discard; i < total; i++) {
        _set(stk_ad[i] / stk_total[i], i);
    }
}


Indicator HKU_API ADVANCE(const DatetimeList& ref, const string& market, int stk_type) {
    IndicatorImpPtr p = make_shared<IAdvance>();
    p->setParam<DatetimeList>("align_date_list", ref);
    p->setParam<string>("market", market);
    p->setParam<int>("stk_type", stk_type);
    return Indicator(p);
}


} /* namespace hku */
