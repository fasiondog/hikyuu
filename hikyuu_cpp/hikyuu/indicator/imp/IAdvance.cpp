/*
 * IAdvance.cpp
 * 
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-20
 *      Author: fasiondog
 */

#include "IAdvance.h"
#include "../../StockManager.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAdvance)
#endif


namespace hku {

IAdvance::IAdvance() : IndicatorImp("ADVANCE", 1) {
    setParam<KQuery>("query", KQueryByIndex(-100));
    setParam<string>("market", "SH");
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
    StockManager& sm = StockManager::instance();

    DatetimeList dates = sm.getTradingCalendar(q, market);
    for (auto iter = sm.begin(); iter != sm.end(); ++iter) {
        KData k = iter->getKData(q);
    }    


}


Indicator HKU_API ADVANCE(const DatetimeList& ref) {
    IndicatorImpPtr p = make_shared<IAdvance>();
    p->setParam<DatetimeList>("align_date_list", ref);
    return Indicator(p);
}


} /* namespace hku */
