/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/CVAL.h"
#include "hikyuu/indicator/crt/SLICE.h"
#include "IContext.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IContext)
#endif

namespace hku {

IContext::IContext() : IndicatorImp("CONTEXT") {
    m_need_self_alike_compare = true;
    setParam<bool>("fill_null", false);
    setParam<bool>("use_self_ktype", false);         // 使用自身独立上下文的K线类型
    setParam<bool>("use_self_recover_type", false);  // 使用自身独立上下文的复权类型
}

IContext::IContext(const Indicator& ref_ind) : IndicatorImp("CONTEXT"), m_ref_ind(ref_ind) {
    m_need_self_alike_compare = true;
    setParam<bool>("fill_null", false);
    setParam<bool>("use_self_ktype", false);
    setParam<bool>("use_self_recover_type", false);
}

IContext::~IContext() {}

IndicatorImpPtr IContext::_clone() {
    auto p = make_shared<IContext>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

void IContext::_clearIntermediateResults() {
    m_ref_ind.clearIntermediateResults();
}

string IContext::str() const {
    std::ostringstream os;
    os << "Indicator{\n"
       << "  context: " << m_ref_ind.getContext().getStock().market_code() << "\n  name: " << name()
       << "\n  size: " << size() << "\n  discard: " << discard()
       << "\n  result sets: " << getResultNumber() << "\n  params: " << getParameter();
    os << "\n  formula: " << formula();
    for (size_t r = 0; r < getResultNumber(); ++r) {
        if (m_pBuffer[r]) {
            os << "\n  values" << r << ": " << *m_pBuffer[r];
        }
    }
    os << "\n}";
    return os.str();
}

string IContext::formula() const {
    return fmt::format("CONTEXT({})", m_ref_ind.formula());
}

KData IContext::getContextKdata() const {
    return m_ref_ind.getContext();
}

bool IContext::selfAlike(const IndicatorImp& other) const noexcept {
    const auto& other_ctx = dynamic_cast<const IContext&>(other);
    return m_ref_ind.getImp()->alike(*other_ctx.m_ref_ind.getImp());
}

void IContext::_calculate(const Indicator& ind) {
    HKU_ASSERT(isLeaf());

    auto null_k = Null<KData>();
    const auto& in_k = getContext();
    auto self_k = m_ref_ind.getContext();
    HKU_IF_RETURN((self_k == in_k || in_k == null_k) && this->size() != 0, void());

    auto self_dates = m_ref_ind.getDatetimeList();
    // HKU_WARN_IF((self_k == null_k && m_ref_ind.empty() && self_dates.empty()),
    //             "The data length of context is zero! ");

    auto ref = m_ref_ind;

    if (in_k != null_k && in_k != self_k) {
        if (self_dates.empty() && self_k.getStock().isNull()) {
            // 上下文无效且无对齐日期，按时间无关序列计算并对齐
            if (ref.size() > in_k.size()) {
                ref = SLICE(ref, ref.size() - in_k.size(), ref.size());
            } else if (ref.size() < in_k.size()) {
                // 右对齐
                ref = CVAL(0.)(in_k) + ref;
            }  // else 长度相等无需再处理

        } else if (self_k != null_k) {
            // 如果参考指标是时间序列，自按当前上下文日期查询条件查询后按日期对齐
            bool use_self_ktype = getParam<bool>("use_self_ktype");
            bool use_self_recover_type = getParam<bool>("use_self_recover_type");
            auto self_stk = self_k.getStock();
            if (use_self_ktype || use_self_recover_type) {
                const auto& self_query = self_k.getQuery();
                const auto& in_query = in_k.getQuery();
                auto ktype = use_self_ktype ? self_query.kType() : in_query.kType();
                auto recover_type =
                  use_self_recover_type ? self_query.recoverType() : in_query.recoverType();
                KQuery query;
                if (in_query.queryType() == KQuery::DATE) {
                    query = KQueryByDate(in_query.startDatetime(), in_query.endDatetime(), ktype,
                                         recover_type);
                } else {
                    query = KQueryByIndex(in_query.start(), in_query.end(), ktype, recover_type);
                }
                ref = m_ref_ind(self_stk.getKData(query));

            } else {
                ref = m_ref_ind(self_stk.getKData(in_k.getQuery()));
            }
            ref = ALIGN(ref, in_k, getParam<bool>("fill_null"));
        } else if (self_dates.size() > 1) {
            // 无上下文的时间序列
            ref = ALIGN(ref, in_k, getParam<bool>("fill_null"));
        }
    }

    size_t total = ref.size();
    size_t rtotal = ref.getResultNumber();
    _readyBuffer(total, rtotal);

    m_discard = ref.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t len = sizeof(value_t) * (total - m_discard);
    for (size_t r = 0; r < rtotal; ++r) {
        const auto* src = ref.data(r) + m_discard;
        auto* dst = this->data(r) + m_discard;
        memcpy(dst, src, len);
    }
}

Indicator HKU_API CONTEXT(bool fill_null, bool use_self_ktype, bool use_self_recover_type) {
    auto p = make_shared<IContext>();
    p->setParam<bool>("fill_null", fill_null);
    p->setParam<bool>("use_self_ktype", use_self_ktype);
    p->setParam<bool>("use_self_recover_type", use_self_recover_type);
    return Indicator(p);
}

Indicator HKU_API CONTEXT(const Indicator& ind, bool fill_null, bool use_self_ktype,
                          bool use_self_recover_type) {
    auto p = make_shared<IContext>(ind);
    p->setParam<bool>("fill_null", fill_null);
    p->setParam<bool>("use_self_ktype", use_self_ktype);
    p->setParam<bool>("use_self_recover_type", use_self_recover_type);
    return p->calculate();
}

KData HKU_API CONTEXT_K(const Indicator& ind) {
    auto imp = ind.getImp();
    IContext const* p = dynamic_cast<IContext const*>(imp.get());
    if (p != nullptr) {
        return p->getContextKdata();
    }
    return ind.getContext();
}

}  // namespace hku