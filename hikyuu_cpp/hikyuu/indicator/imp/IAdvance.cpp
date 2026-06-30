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
    setParam<bool>("ignore_context", false);
    setParam<bool>("fill_null", true);
}

IAdvance::~IAdvance() {}

void IAdvance::_checkParam(const string& name) const {
    if ("market" == name) {
        string market = getParam<string>(name);
        auto market_info = StockManager::instance().getMarketInfo(market);
        HKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}", market);
    } else if ("stk_type" == name) {
        int stk_type = getParam<int>("stk_type");
        HKU_ASSERT(stk_type >= 0);
    }
}

void IAdvance::_calculate(const Indicator& ind) {
    // ref_date_list 参数会影响 IndicatorImp 全局，勿随意修改
    string market;
    KQuery q;
    int stk_type = STOCKTYPE_A;

    StockManager& sm = StockManager::instance();
    DatetimeList dates;

    bool ignore_context = getParam<bool>("ignore_context");
    const KData& k = getContext();
    if (!ignore_context && k.getStock().type() != STOCKTYPE_INDEX) {
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

    m_discard = 1;
    _readyBuffer(total, 1);

    // 需要将 Query 转换为 KQueryByDate
    q = KQueryByDate(dates.front(), dates.back() + Seconds(KQuery::getKTypeInSeconds(q.kType())),
                     q.kType(), q.recoverType());

    auto* dst = this->data();
    Indicator x = ALIGN(CLOSE() > REF(CLOSE(), 1), std::move(dates), getParam<bool>("fill_null"));
    for (auto iter = sm.begin(); iter != sm.end(); ++iter) {
        if ((stk_type <= STOCKTYPE_TMP && iter->type() != stk_type) ||
            (market != "" && iter->market() != market)) {
            continue;
        }
        x.setContext(*iter, q);
        auto const* xdata = x.data();
        if (x.empty()) {
            continue;
        }
        for (size_t i = x.discard(); i < x.size(); i++) {
            if (x.getDatetime(i) > iter->lastDatetime()) {
                break;
            }

            if (!std::isnan(xdata[i]) && xdata[i] > 0.0) {
                dst[i] = std::isnan(dst[i]) ? 1 : dst[i] + 1;
            }
        }
    }
}

bool IAdvance::supportIncrementCalculate() const {
    bool ignore_context = getParam<bool>("ignore_context");
    const KData& k = getContext();
    return !ignore_context && k.getStock().type() != STOCKTYPE_INDEX;
}

void IAdvance::_increment_calculate(const Indicator& data, size_t start_pos) {
    SPEND_TIME(_increment_calculate);
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
    // 强制清理将要重算的 dst 区间, 防止 increment_execute_leaf_or_op 拷贝过来的
    // 旧汇总值被全市场遍历二次累加(脏读双累加: 旧值非 NaN 时 isnan?1:+1 会再 +1)。
    // 从 start_pos 起清(而非 start_pos-1): start_pos-1 是旧窗口最后一根的正确汇总,
    // 增量内层循环从 x.discard()>=1 起写 dst[i+start_pos-1], 最小写 dst[start_pos],
    // 从不写 dst[start_pos-1], 故保留其旧值。
    for (size_t i = start_pos; i < this->size(); ++i) {
        dst[i] = Null<value_t>();
    }
    Indicator x = ALIGN(CLOSE() > REF(CLOSE(), 1), std::move(dates), getParam<bool>("fill_null"));
    for (auto iter = sm.begin(); iter != sm.end(); ++iter) {
        if ((stk_type <= STOCKTYPE_TMP && iter->type() != stk_type) ||
            (market != "" && iter->market() != market)) {
            continue;
        }
        x.setContext(*iter, q);
        auto const* xdata = x.data();
        if (x.empty()) {
            continue;
        }
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

Indicator HKU_API ADVANCE(const KQuery& query, const string& market, int stk_type,
                          bool ignore_context, bool fill_null) {
    IndicatorImpPtr p = make_shared<IAdvance>();
    p->setParam<KQuery>("query", query);
    p->setParam<string>("market", market);
    p->setParam<int>("stk_type", stk_type);
    p->setParam<bool>("ignore_context", ignore_context);
    p->setParam<bool>("fill_null", fill_null);
    p->calculate();
    return Indicator(p);
}

} /* namespace hku */
