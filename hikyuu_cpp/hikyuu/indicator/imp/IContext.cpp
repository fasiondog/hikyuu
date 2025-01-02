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
    setParam<bool>("fill_null", true);
}

IContext::IContext(const Indicator& ref_ind) : IndicatorImp("CONTEXT"), m_ref_ind(ref_ind) {
    setParam<bool>("fill_null", true);
}

IContext::~IContext() {}

IndicatorImpPtr IContext::_clone() {
    auto p = make_shared<IContext>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

string IContext::str() const {
    std::ostringstream os;
    os << "Indicator{\n"
       << "  context: " << m_ref_ind.getContext().getStock().market_code() << "\n  name: " << name()
       << "\n  size: " << size() << "\n  discard: " << discard()
       << "\n  result sets: " << getResultNumber() << "\n  params: " << getParameter()
       << "\n  support indicator param: " << (supportIndParam() ? "True" : "False");
    if (supportIndParam()) {
        os << "\n  ind params: {";
        const auto& ind_params = getIndParams();
        for (auto iter = ind_params.begin(); iter != ind_params.end(); ++iter) {
            os << iter->first << ": " << iter->second->formula() << ", ";
        }
        os << "}";
    }
    os << "\n  formula: " << formula();
#if !HKU_USE_LOW_PRECISION
    if (m_pBuffer[0]) {
        os << "\n  values: " << *m_pBuffer[0];
    }
#endif
    os << "\n}";
    return os.str();
}

string IContext::formula() const {
    return fmt::format("CONTEXT({})", m_ref_ind.formula());
}

KData IContext::getContextKdata() const {
    return m_ref_ind.getContext();
}

void IContext::_calculate(const Indicator& ind) {
    HKU_ASSERT(isLeaf());

    auto null_k = Null<KData>();
    auto in_k = getContext();
    auto self_k = m_ref_ind.getContext();
    HKU_IF_RETURN((self_k == in_k || in_k == null_k) && this->size() != 0, void());

    auto self_dates = m_ref_ind.getDatetimeList();
    HKU_WARN_IF((self_k == null_k && m_ref_ind.empty() && self_dates.empty()),
                "The data length of context is zero! ");

    auto ref = m_ref_ind;

    if (in_k != null_k && in_k != self_k) {
        if (self_dates.empty()) {
            // 上下文无效且无对齐日期，按时间无关序列计算并对齐
            if (ref.size() > in_k.size()) {
                ref = SLICE(ref, ref.size() - in_k.size(), ref.size());
            } else if (ref.size() < in_k.size()) {
                // 右对齐
                ref = CVAL(0.)(in_k) + ref;
            }  // else 长度相等无需再处理
        } else if (self_k != null_k) {
            // 如果参考指标是时间序列，自按当前上下文日期查询条件查询后按日期对齐
            auto self_stk = self_k.getStock();
            ref = m_ref_ind(self_stk.getKData(in_k.getQuery()));
            ref = ALIGN(ref, in_k, getParam<bool>("fill_null"));
        } else if (self_dates.size() > 1) {
            // 无上下文的时间序列
            ref = ALIGN(ref, in_k, getParam<bool>("fill_null"));
        }
    }

    size_t total = ref.size();
    _readyBuffer(total, ref.getResultNumber());

    m_discard = ref.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    size_t rtotal = ref.getResultNumber();
    _readyBuffer(total, rtotal);
    size_t len = sizeof(value_t) * (total - m_discard);
    for (size_t r = 0; r < rtotal; ++r) {
        const auto* src = ref.data(r) + m_discard;
        auto* dst = this->data(r) + m_discard;
        memcpy(dst, src, len);
    }
}

Indicator HKU_API CONTEXT(bool fill_null) {
    auto p = make_shared<IContext>();
    p->setParam<bool>("fill_null", fill_null);
    return Indicator(p);
}

Indicator HKU_API CONTEXT(const Indicator& ind, bool fill_null) {
    auto p = make_shared<IContext>(ind);
    p->setParam<bool>("fill_null", fill_null);
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