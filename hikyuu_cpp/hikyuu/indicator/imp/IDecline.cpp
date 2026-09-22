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
    setParam<bool>("fill_null", true);
}

IDecline::~IDecline() {}

void IDecline::_checkParam(const string& name) const {
    if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    } else if ("stk_type" == name) {
        int stk_type = getParam<int>("stk_type");
        HKU_ASSERT(stk_type >= 0);
    }
}

void IDecline::_calculate(const Indicator& ind) {
    // The ref_date_list parameter affects the IndicatorImp globally, do not modify it at will
    string market;
    KQuery q;
    int stk_type = STOCKTYPE_A;

    StockManager& sm = StockManager::instance();
    DatetimeList dates;

    bool ignore_context = getParam<bool>("ignore_context");
    const KData& k = getContext();
    if (!ignore_context && !k.empty() && k.getStock().type() != STOCKTYPE_INDEX) {
        q = k.getQuery();
        Stock stk = k.getStock();
        market = stk.market();
        stk_type = stk.type();
        dates = k.getDatetimeList();
    } else {
        market = getParam<string>("market");
        q = getParam<KQuery>("query");
        stk_type = getParam<int>("stk_type");
        dates = sm.getTradingCalendar(q, market);
    }

    size_t total = dates.size();
    if (total == 0) {
        m_discard = 0;
        _readyBuffer(0, 1);
        return;
    }

    // The Query needs to be converted into KQueryByDate
    q = KQueryByDate(dates.front(), dates.back() + Seconds(KQuery::getKTypeInSeconds(q.kType())),
                     q.kType(), q.recoverType());

    m_discard = 1;
    _readyBuffer(total, 1);
    auto* dst = this->data();
    Indicator x = ALIGN(CLOSE() < REF(CLOSE(), 1), std::move(dates), getParam<bool>("fill_null"));
    for (auto iter = sm.begin(); iter != sm.end(); ++iter) {
        if ((stk_type <= STOCKTYPE_TMP && iter->type() != stk_type) ||
            (market != "" && iter->market() != market)) {
            continue;
        }
        x.setContext(*iter, q);
        if (x.empty()) {
            continue;
        }
        auto const* xdata = x.data();
        for (size_t i = x.discard(); i < total; i++) {
            if (x.getDatetime(i) > iter->lastDatetime()) {
                break;
            }

            if (!std::isnan(xdata[i]) && xdata[i] > 0.0) {
                dst[i] = std::isnan(dst[i]) ? 1 : dst[i] + 1;
            }
        }
    }
}

bool IDecline::supportIncrementCalculate() const {
    bool ignore_context = getParam<bool>("ignore_context");
    const KData& k = getContext();
    return !ignore_context && k.getStock().type() != STOCKTYPE_INDEX;
}

void IDecline::_increment_calculate(const Indicator& data, size_t start_pos) {
    const auto& k = getContext();
    auto q = k.getQuery();
    auto stk = k.getStock();
    const auto& market = stk.market();
    auto stk_type = stk.type();
    DatetimeList old_dates = k.getDatetimeList();
    DatetimeList dates;
    for (size_t i = start_pos - 1; i < old_dates.size(); i++) {
        dates.push_back(old_dates[i]);
    }
    q = KQueryByDate(dates.front(), dates.back() + Seconds(KQuery::getKTypeInSeconds(q.kType())),
                     q.kType(), q.recoverType());

    StockManager& sm = StockManager::instance();
    auto* dst = this->data();
    // Force clearing the dst range that is about to be recalculated, to prevent the old summary
    // value copied by increment_execute_leaf_or_op from being accumulated twice by the whole
    // market traversal (a dirty read double accumulation: when the old value is not NaN,
    // isnan ? 1 : +1 adds one more). The clearing starts from start_pos (not start_pos-1)
    // because start_pos-1 is the correct summary of the last bar of the old window; the
    // incremental inner loop writes dst[i+start_pos-1] starting from x.discard()>=1, writing
    // dst[start_pos] at the earliest, and never writes dst[start_pos-1], so its old value is
    // kept.
    for (size_t i = start_pos; i < this->size(); ++i) {
        dst[i] = Null<value_t>();
    }
    Indicator x = ALIGN(CLOSE() < REF(CLOSE(), 1), std::move(dates), getParam<bool>("fill_null"));
    for (auto iter = sm.begin(); iter != sm.end(); ++iter) {
        if ((stk_type <= STOCKTYPE_TMP && iter->type() != stk_type) ||
            (market != "" && iter->market() != market)) {
            continue;
        }
        x.setContext(*iter, q);
        if (x.empty()) {
            continue;
        }
        auto const* xdata = x.data();
        for (size_t i = x.discard(); i < x.size(); i++) {
            if (x.getDatetime(i) > iter->lastDatetime()) {
                break;
            }

            if (!std::isnan(xdata[i]) && xdata[i] > 0.0) {
                dst[i + start_pos - 1] =
                  std::isnan(dst[i + start_pos - 1]) ? 1 : dst[i + start_pos - 1] + 1;
            }
        }
    }
}

Indicator HKU_API DECLINE(const KQuery& query, const string& market, int stk_type,
                          bool ignore_context, bool fill_null) {
    IndicatorImpPtr p = make_shared<IDecline>();
    p->setParam<KQuery>("query", query);
    p->setParam<string>("market", market);
    p->setParam<int>("stk_type", stk_type);
    p->setParam<bool>("ignore_context", ignore_context);
    p->setParam<bool>("fill_null", fill_null);
    p->calculate();
    return Indicator(p);
}

} /* namespace hku */
