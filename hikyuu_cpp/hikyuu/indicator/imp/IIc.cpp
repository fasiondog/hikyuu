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
#include "IIc.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IIc)
#endif

namespace hku {

IIc::IIc() : IndicatorImp("IC", 1) {
    setParam<int>("n", 1);  // 调仓周期
    // 对齐时是否以 nan 值进行填充，否则以小于当前日期的最后值作为填充
    setParam<bool>("fill_null", true);
}

IIc::IIc(const StockList& stks, const KQuery& query, int n, const Stock& ref_stk)
: IndicatorImp("IC", 1), m_query(query), m_ref_stk(ref_stk), m_stks(stks) {
    setParam<int>("n", n);
    setParam<bool>("fill_null", true);
}

IIc::~IIc() {}

void IIc::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 1);
    }
}

IndicatorImpPtr IIc::_clone() {
    IIc* p = new IIc();
    p->m_stks = m_stks;
    p->m_query = m_query;
    p->m_ref_stk = m_ref_stk;
    return IndicatorImpPtr(p);
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
    // 由于 ic 是当前收盘价和 n 日后收益的相关系数，需要避免未来函数，最终的 ic 需要右移 n 日
    // spearman 本身需要数据长度大于等于2，所以 n + 1 >= days_totals 时，直接抛弃
    HKU_IF_RETURN(n + 1 >= days_total, void());

    bool fill_null = getParam<bool>("fill_null");

    // 计算每支证券对齐后的因子值与 n 日收益率
    vector<Indicator> all_inds(stk_count);     // 保存每支证券对齐后的因子值
    vector<Indicator> all_returns(stk_count);  // 保存每支证券对齐后的 n 日收益率
    Indicator ind = inputInd;
    size_t discard = n;
    for (size_t i = 0; i < stk_count; i++) {
        auto k = m_stks[i].getKData(m_query);
        all_inds[i] = ALIGN(ind(k), ref_dates, fill_null);
        // 计算 n 日收益率，同时需要右移 n 位，即第 i 日的因子值和第 i + n 的收益率对应
        all_returns[i] = ALIGN(REF(ROCP(k.close(), n), n), ref_dates, fill_null);
    }

    m_discard = discard;
    HKU_IF_RETURN(m_discard >= days_total, void());

    PriceList tmp(stk_count, Null<price_t>());
    PriceList tmp_return(stk_count, Null<price_t>());
    auto* dst = this->data();
    for (size_t i = m_discard; i < days_total; i++) {
        // 计算日截面 spearman 相关系数即 ic 值
        for (size_t j = 0; j < stk_count; j++) {
            tmp[j] = all_inds[j][i];
            tmp_return[j] = all_returns[j][i];
        }
        auto a = PRICELIST(tmp);
        auto b = PRICELIST(tmp_return);
        auto ic = hku::SPEARMAN(a, b, stk_count);
        dst[i] = ic[ic.size() - 1];
    }

    for (size_t i = m_discard; i < days_total; i++) {
        if (!std::isnan(dst[i])) {
            m_discard = i;
            break;
        }
    }
}

Indicator HKU_API IC(const StockList& stks, const KQuery& query, const Stock& ref_stk, int n) {
    return Indicator(make_shared<IIc>(stks, query, n, ref_stk));
}

Indicator HKU_API IC(const Block& blk, const KQuery& query, const Stock& ref_stk, int n) {
    StockList stks = blk.getStockList();
    return IC(stks, query, ref_stk, n);
}

}  // namespace hku