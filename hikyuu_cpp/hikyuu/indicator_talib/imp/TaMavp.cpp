/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>
#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/CVAL.h"
#include "hikyuu/indicator/crt/SLICE.h"
#include "hikyuu/indicator/crt/PRICELIST.h"
#include "hikyuu/indicator/imp/IContext.h"
#include "hikyuu/indicator/crt/CONTEXT.h"
#include "TaMavp.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::TaMavp)
#endif

namespace hku {

TaMavp::TaMavp() : IndicatorImp("TA_MAVP", 1) {
    setParam<int>("min_n", 2);
    setParam<int>("max_n", 30);
    setParam<int>("matype", 0);
}

TaMavp::TaMavp(const Indicator& ref_ind, int min_n, int max_n, int matype)
: IndicatorImp("TA_MAVP", 1), m_ref_ind(ref_ind) {
    setParam<int>("min_n", min_n);
    setParam<int>("max_n", max_n);
    setParam<int>("matype", matype);
    m_name = fmt::format("TA_MAVP({})", ref_ind.formula());
}

TaMavp::~TaMavp() {}

void TaMavp::_checkParam(const string& name) const {
    if ("min_n" == name || "max_n" == name) {
        int n = getParam<int>(name);
        HKU_CHECK(n >= 2 && n <= 100000, "{} must in [2, 100000]", name);
    } else if (name == "matype") {
        int matype = getParam<int>(name);
        HKU_ASSERT(matype >= 0 && matype <= 8);
    }
}

IndicatorImpPtr TaMavp::_clone() {
    auto p = make_shared<TaMavp>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

void TaMavp::_calculate(const Indicator& ind) {
#if 1
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    KData in_k = ind.getContext();
    KData ref_k = m_ref_ind.getContext();
    KData null_k = Null<KData>();

    Indicator ref = m_ref_ind;

    // 可能是原型公式
    if (ref.empty() && ref_k == null_k) {
        ref = m_ref_ind(in_k);
        if (!ref.empty()) {
            ref_k = in_k;
        } else {
            ref = m_ref_ind;
        }
    }

    DatetimeList in_dates = ind.getDatetimeList();
    DatetimeList ref_dates = ref.getDatetimeList();

    if ((in_k == null_k && in_dates.empty()) || (ref_k == null_k && ref_dates.empty())) {
        // 上下文无效且无对齐日期，按时间无关序列计算并对齐
        if (ref.size() > ind.size()) {
            HKU_INFO("1");
            ref = SLICE(ref, ref.size() - ind.size(), ref.size());
        } else if (ref.size() < ind.size()) {
            // 右对齐
            ref = CVAL(ind, 0.) + ref;
            HKU_INFO("2");
            // HKU_INFO("ind: {}, ref: {}", ind, ref);
        } else {  // else 长度相等无需再处理
            HKU_INFO("3");
        }
    } else if (ref_k != null_k && ref_k != in_k) {
        // 参考指标为独立的上下文，则使用输入上下文的查询条件
        HKU_INFO("4");
        auto ref_stk = ref_k.getStock();
        ref = m_ref_ind(ref_stk.getKData(in_k.getQuery()));
        ref = ALIGN(ref, in_k, false);
    } else if (ref_k.size() != ref.size()) {
        // ref_k 和 ref 长度不相等，ref是独立的时间序列
        HKU_INFO("5");
        ref = ALIGN(ref, in_k);
    } else {
        HKU_INFO("6");
    }

#else
    auto k = getContext();
    m_ref_ind.setContext(k);
    Indicator ref = m_ref_ind;
    auto dates = ref.getDatetimeList();
    if (dates.empty()) {
        if (ref.size() > ind.size()) {
            ref = SLICE(ref, ref.size() - ind.size(), ref.size());
        } else if (ref.size() < ind.size()) {
            ref = CVAL(ind, 0.) + ref;
        }
    } else if (m_ref_ind.size() != ind.size()) {
        ref = ALIGN(m_ref_ind, ind);
    }

    size_t total = ind.size();
    _readyBuffer(total, 2);
    HKU_IF_RETURN(total == 0, void());
#endif

    int min_n = getParam<int>("min_n");
    int max_n = getParam<int>("max_n");
    TA_MAType matype = (TA_MAType)getParam<int>("matype");
    int lookback = TA_MAVP_Lookback(min_n, max_n, matype);
    if (lookback < 0) {
        m_discard = total;
        return;
    }

    m_discard = lookback + std::max(ind.discard(), ref.discard());
    HKU_INFO("m_discard: {}, lookback: {}", m_discard, lookback);
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    const auto* src0 = ind.data();
    const auto* src1 = ref.data();
    auto* dst = this->data();
    int outBegIdx;
    int outNbElement;
    TA_MAVP(m_discard, total - 1, src0, src1, min_n, max_n, matype, &outBegIdx, &outNbElement,
            dst + m_discard);
    HKU_ASSERT((outBegIdx == m_discard) && (outBegIdx + outNbElement) <= total);
}

Indicator HKU_API TA_MAVP(const Indicator& ref_ind, int min_n, int max_n, int matype) {
#if 1
    return Indicator(make_shared<TaMavp>(ref_ind, min_n, max_n, matype));
#else
    auto imp = ref_ind.getImp();
    if (imp) {
        IContext* p = dynamic_cast<IContext*>(imp.get());
        if (p == nullptr) {
            return Indicator(make_shared<TaMavp>(CONTEXT(ref_ind), min_n, max_n, matype));
        }
    }
    return Indicator(make_shared<TaMavp>(ref_ind, min_n, max_n, matype));
#endif
}

}  // namespace hku