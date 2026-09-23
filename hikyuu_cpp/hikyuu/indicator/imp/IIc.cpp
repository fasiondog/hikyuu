/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-09
 *      Author: fasiondog
 */

#include "hikyuu/Block.h"
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/REF.h"
#include "hikyuu/indicator/crt/ROCP.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/crt/SPEARMAN.h"
#include "hikyuu/indicator/crt/CORR.h"
#include "hikyuu/indicator/crt/KDATA.h"
#include "IIc.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIc)
#endif

namespace hku {

IIc::IIc() : IndicatorImp("IC", 1) {
    m_need_self_alike_compare = true;
    setParam<int>("n", 1);  // Position adjustment cycle
    // Whether to fill with nan during the alignment, otherwise the last value earlier than the
    // current date is used as the fill
    setParam<bool>("fill_null", true);
    setParam<bool>("use_spearman",
                   true);  // Use SPEARMAN by default, otherwise the pearson correlation

    // Strict IC calculation, i.e. fill NA when the future return is unknown; it is equivalent to
    // shifting left by n in the non-strict case
    setParam<bool>("strict", false);
}

IIc::IIc(const StockList& stks, int n, bool spearman, bool strict)
: IndicatorImp("IC", 1), m_stks(stks) {
    m_need_self_alike_compare = true;
    setParam<int>("n", n);
    setParam<bool>("fill_null", true);
    setParam<bool>("use_spearman", spearman);
    setParam<bool>("strict", strict);
}

IIc::~IIc() {}

void IIc::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 1);
    }
}

IndicatorImpPtr IIc::_clone() {
    auto p = make_shared<IIc>();
    p->m_stks = m_stks;
    return p;
}

bool IIc::selfAlike(const IndicatorImp& other) const noexcept {
    const auto* other_ind = dynamic_cast<const IIc*>(&other);
    HKU_IF_RETURN(other_ind == nullptr, false);
    HKU_IF_RETURN(other_ind->m_stks.size() != m_stks.size(), false);
    std::unordered_set<string> names;
    names.reserve(m_stks.size());
    for (const auto& stk : m_stks) {
        names.insert(stk.market_code());
    }
    for (const auto& stk : other_ind->m_stks) {
        if (names.find(stk.market_code()) == names.end()) {
            return false;
        }
    }
    return true;
}

void IIc::_calculate(const Indicator& inputInd) {
    // Allocate the memory first, keeping the same length as the reference dates
    auto ref_dates = getContext().getDatetimeList();
    size_t days_total = ref_dates.size();
    _readyBuffer(days_total, 1);

    // Detect the abnormal input data
    m_discard = days_total;
    HKU_IF_RETURN(days_total < 2, void());

    size_t stk_count = m_stks.size();
    HKU_ERROR_IF_RETURN(stk_count < 2, void(),
                        "The number(>=2) of stock is insufficient! current stock number: {}",
                        stk_count);
    for (size_t i = 0; i < stk_count; i++) {
        HKU_ERROR_IF_RETURN(m_stks[i].isNull(), void(), "The [{}] stock is null!", i);
    }

    int n = getParam<int>("n");
    HKU_IF_RETURN(n >= days_total, void());

    bool fill_null = getParam<bool>("fill_null");

    // Calculate the aligned factor value and the n-day return of every security
    vector<Indicator> all_inds(stk_count);     // Save the aligned factor value of every security
    vector<Indicator> all_returns(stk_count);  // Save the aligned n-day return of every security

    KQuery query = getContext().getQuery();

    // Note: using the excess return and the absolute return gives the same final result (they are
    // identical in theory; an extra excess return calculation only introduces a negligible
    // calculation error)
    global_parallel_for_index_void(
      0, stk_count, [&, n, fill_null, ind = inputInd.clone()](size_t i) {
          auto k = m_stks[i].getKData(query);
          // Suppose IC originally needs "the factor value at t -> the return at t+1"; it is changed
          // to calculate "the factor value at t -> the return of the N days before t" (such as the
          // return of the past 5 days), which is called the "current IC". (Otherwise the current
          // values would all be missing NA.)
          all_inds[i] = ALIGN(REF(ind, n), ref_dates, fill_null)(k).getResult(0)();

          // Calculate the absolute return
          all_returns[i] = ALIGN(ROCP(CLOSE(), n), ref_dates, fill_null)(k).getResult(0)();
      });

    m_discard = n;

    Indicator (*spearman)(const Indicator&, const Indicator&, int, bool) = hku::SPEARMAN;
    if (!getParam<bool>("use_spearman")) {
        spearman = hku::CORR;
    }

    auto* dst = this->data();
    global_parallel_for_index_void(m_discard, days_total, [&, stk_count, dst](size_t i) {
        // Calculate the daily cross-sectional spearman correlation coefficient, i.e. the IC value
        PriceList tmp(stk_count, Null<price_t>());
        PriceList tmp_return(stk_count, Null<price_t>());
        for (size_t j = 0; j < stk_count; j++) {
            tmp[j] = all_inds[j][i];
            tmp_return[j] = all_returns[j][i];
        }
        auto a = PRICELIST(tmp);
        auto b = PRICELIST(tmp_return);
        auto ic = spearman(a, b, stk_count, true);
        if (ic.size() > 0) {
            dst[i] = ic[ic.size() - 1];
        }
    });

    if (getParam<bool>("strict")) {
        // The strict mode, i.e. the calculation result of the current moment corresponding to the
        // future return
        for (size_t i = m_discard; i < days_total; i++) {
            dst[i - n] = dst[i];
        }
        if (days_total > n) {
            for (size_t i = days_total - n; i < days_total; i++) {
                dst[i] = Null<price_t>();
            }
        }
        m_discard = 0;
    }

    updateDiscard();
}

Indicator HKU_API IC(const StockList& stks, int n, bool spearman, bool strict) {
    return Indicator(make_shared<IIc>(stks, n, spearman, strict));
}

Indicator HKU_API IC(const Block& blk, int n, bool spearman, bool strict) {
    StockList stks = blk.getStockList();
    return IC(stks, n, spearman, strict);
}

Indicator HKU_API IC(IndicatorList inds, IndicatorList returns, int n, bool use_spearman,
                     bool strict) {
    HKU_CHECK(n >= 1, "The n({}) must be greater than 1!", n);
    HKU_CHECK(inds.size() == returns.size(),
              "The number({}) of indicators is not equal to the number({}) of returns!",
              inds.size(), returns.size());

    size_t stk_count = inds.size();
    HKU_CHECK(stk_count >= 2, "The number of indicators is less than 2!");

    size_t days_total = inds[0].size();
    HKU_CHECK(days_total >= 2, "The size of ind is less than 2!");

    for (size_t i = 0; i < stk_count; i++) {
        HKU_CHECK(
          inds[i].size() == days_total,
          "{}: The number of days({}) is not equal to the number of days({}) of indicators!", i,
          days_total, inds[i].size());
        HKU_CHECK(returns[i].size() == days_total,
                  "{} The number of days({}) is not equal to the number of days({}) of returns!", i,
                  days_total, returns[i].size());
    }

    Indicator (*spearman)(const Indicator&, const Indicator&, int, bool) = hku::SPEARMAN;
    if (!use_spearman) {
        spearman = hku::CORR;
    }

    IndicatorList ref_inds(stk_count);
    global_parallel_for_index_void(0, stk_count, [&](size_t i) { ref_inds[i] = REF(inds[i], n); });

    PriceList ret(days_total, Null<price_t>());
    auto* dst = ret.data();
    global_parallel_for_index_void(0, days_total, [&, stk_count, dst](size_t i) {
        // Calculate the daily cross-sectional spearman correlation coefficient, i.e. the IC value
        PriceList tmp(stk_count, Null<price_t>());
        PriceList tmp_return(stk_count, Null<price_t>());
        for (size_t j = 0; j < stk_count; j++) {
            tmp[j] = ref_inds[j][i];
            tmp_return[j] = returns[j][i];
        }
        auto a = PRICELIST(tmp);
        auto b = PRICELIST(tmp_return);
        auto ic = spearman(a, b, stk_count, true);
        if (ic.size() > 0) {
            dst[i] = ic[ic.size() - 1];
        }
    });

    if (strict) {
        // The strict mode, i.e. the calculation result of the current moment corresponding to the
        // future return
        for (size_t i = n; i < days_total; i++) {
            dst[i - n] = dst[i];
        }
        if (days_total > n) {
            for (size_t i = days_total - n; i < days_total; i++) {
                dst[i] = Null<price_t>();
            }
        }
    }

    size_t discard = days_total;
    for (size_t i = 0; i < days_total; i++) {
        if (!std::isnan(dst[i])) {
            discard = i;
            break;
        }
    }

    return PRICELIST(ret, discard);
}

}  // namespace hku