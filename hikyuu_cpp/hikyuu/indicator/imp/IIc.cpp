/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-09
 *      Author: fasiondog
 */

#include "hikyuu/Block.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/ROCP.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/SPEARMAN.h"
#include "IIc.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIc)
#endif

namespace hku {

IIc::IIc() : IndicatorImp("IC") {
    setParam<int>("n", 1);
    setParam<bool>("fill_null", true);
}

IIc::IIc(const StockList& stks, const KQuery& query, int n, const Stock& ref_stk)
: IndicatorImp("IC"), m_query(query), m_ref_stk(ref_stk), m_stks(stks) {
    setParam<int>("n", n);
    setParam<bool>("fill_null", true);
}

IIc::~IIc() {}

bool IIc::check() {
    return getParam<int>("n") >= 1;
}

IndicatorImpPtr IIc::_clone() {
    IIc* p = new IIc();
    p->m_stks = m_stks;
    p->m_query = m_query;
    p->m_ref_stk = m_ref_stk;
    return IndicatorImpPtr(p);
}

void IIc::_calculate(const Indicator& inputInd) {
    HKU_ERROR_IF_RETURN(m_ref_stk.isNull(), void(), "ref_stk is null!");
    auto ref_dates = m_ref_stk.getDatetimeList(m_query);
    size_t total = ref_dates.size();
    _readyBuffer(total, 1);

    if (total < 2 || m_stks.size() < 2) {
        HKU_WARN(
          "The number(>=2) of stock or data length(>=2) is insufficient! current data len: {}, "
          "current stock number: {}",
          total, m_stks.size());
        m_discard = total;
    }

    int n = getParam<int>("n");
    bool fill_null = getParam<bool>("fill_null");

    vector<Indicator> all_inds;
    all_inds.reserve(m_stks.size());
    vector<Indicator> all_returns;
    all_returns.reserve(m_stks.size());
    Indicator ind = inputInd;
    for (const auto& stk : m_stks) {
        if (stk.isNull()) {
            HKU_WARN("Exist null stock, it was ignored!");
        } else {
            auto k = stk.getKData(m_query);
            all_inds.push_back(ALIGN(ind(k), ref_dates, fill_null));
            all_returns.push_back(ALIGN(ROCP(k.close(), n), ref_dates, fill_null));
        }
    }

    size_t ind_count = all_inds.size();
    if (all_inds.size() < 2) {
        HKU_WARN("The number of exist stock is insufficient!");
        m_discard = total;
        return;
    }

    PriceList tmp(ind_count, Null<price_t>());
    PriceList tmp_return(ind_count, Null<price_t>());
    auto* dst = this->data();
    for (size_t i = 0; i < total; i++) {
        for (size_t j = 0; j < ind_count; j++) {
            if (i >= all_inds[j].size() || i >= all_returns[j].size()) {
                continue;
            }
            tmp[j] = all_inds[j][i];
            tmp_return[j] = all_returns[j][i];
        }
        auto a = PRICELIST(tmp);
        auto b = PRICELIST(tmp_return);
        auto ic = hku::SPEARMAN(a, b, ind_count);
        dst[i] = ic[ic.size() - 1];
    }
}

Indicator HKU_API IC(const StockList& stks, const KQuery& query, int n, const Stock& ref_stk) {
    return Indicator(make_shared<IIc>(stks, query, n, ref_stk));
}

Indicator HKU_API IC(const Block& blk, const KQuery& query, int n, const Stock& ref_stk) {
    StockList stks = blk.getAllStocks();
    return IC(stks, query, n, ref_stk);
}

}  // namespace hku