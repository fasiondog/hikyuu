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
#include "IIc.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIc)
#endif

namespace hku {

IIc::IIc() : IndicatorImp("IC", 1) {
    setParam<int>("n", 1);  // 调仓周期
    // 对齐时是否以 nan 值进行填充，否则以小于当前日期的最后值作为填充
    setParam<bool>("fill_null", true);
    setParam<bool>("use_spearman", true);  // 默认使用SPEARMAN计算相关系数, 否则使用pearson相关系数

    // 严格IC计算，即未来收益不知道时填充NA，相当于非严格情况下左移n
    setParam<bool>("strict", false);
}

IIc::IIc(const StockList& stks, const KQuery& query, int n, const Stock& ref_stk, bool spearman,
         bool strict)
: IndicatorImp("IC", 1), m_query(query), m_ref_stk(ref_stk), m_stks(stks) {
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
    p->m_query = m_query;
    p->m_ref_stk = m_ref_stk;
    return p;
}

void IIc::_calculate(const Indicator& inputInd) {
    // 先申请内存，保持和参考日期等长
    auto ref_dates = m_ref_stk.getDatetimeList(m_query);
    size_t days_total = ref_dates.size();
    _readyBuffer(days_total, 1);

    // 检测异常输入数据
    m_discard = days_total;
    HKU_ERROR_IF_RETURN(m_ref_stk.isNull(), void(), "ref_stk is null!");
    HKU_ERROR_IF_RETURN(days_total < 2, void(),
                        "The data length(>=2) is insufficient! current data len: {}", days_total);

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

    // 计算每支证券对齐后的因子值与 n 日收益率
    vector<Indicator> all_inds(stk_count);     // 保存每支证券对齐后的因子值
    vector<Indicator> all_returns(stk_count);  // 保存每支证券对齐后的 n 日收益率
    Indicator ind = inputInd;
    global_parallel_for_index_void(0, stk_count, [&, n, fill_null](size_t i) {
        // for (size_t i = 0; i < stk_count; i++) {
        auto k = m_stks[i].getKData(m_query);
        // 假设 IC 原本需要 “t 时刻因子值→t+1 时刻收益”，改为计算 “t 时刻因子值→t 时刻之前 N
        // 天的收益”（比如过去 5 天的收益），并称之为 “当前 IC”。(否则当前值都会是缺失NA)
        all_inds[i] = ALIGN(REF(ind(k), n), ref_dates, fill_null).clearIntermediateResults();
        all_returns[i] = ALIGN(ROCP(k.close(), n), ref_dates, fill_null).clearIntermediateResults();
    });

    m_discard = n;

    Indicator (*spearman)(const Indicator&, const Indicator&, int, bool) = hku::SPEARMAN;
    if (!getParam<bool>("use_spearman")) {
        spearman = hku::CORR;
    }

    // PriceList tmp(stk_count, Null<price_t>());
    // PriceList tmp_return(stk_count, Null<price_t>());
    auto* dst = this->data();
    global_parallel_for_index_void(m_discard, days_total, [&, stk_count, dst](size_t i) {
        // for (size_t i = m_discard; i < days_total; i++) {
        // 计算日截面 spearman 相关系数即 ic 值
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
        // 严格模式，即当前时刻对应未来收益计算结果
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

    for (size_t i = m_discard; i < days_total; i++) {
        if (!std::isnan(dst[i])) {
            m_discard = i;
            break;
        }
    }
}

Indicator HKU_API IC(const StockList& stks, const KQuery& query, const Stock& ref_stk, int n,
                     bool spearman, bool strict) {
    return Indicator(make_shared<IIc>(stks, query, n, ref_stk, spearman, strict));
}

Indicator HKU_API IC(const Block& blk, const KQuery& query, const Stock& ref_stk, int n,
                     bool spearman, bool strict) {
    StockList stks = blk.getStockList();
    return IC(stks, query, ref_stk, n, spearman, strict);
}

}  // namespace hku