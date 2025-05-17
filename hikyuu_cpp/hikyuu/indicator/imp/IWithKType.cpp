/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-28
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "hikyuu/indicator/crt/CVAL.h"
#include "hikyuu/indicator/crt/SLICE.h"
#include "IWithKType.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IWithKType)
#endif

namespace hku {

IWithKType::IWithKType() : IndicatorImp("WITHKTYPE") {
    setParam<bool>("fill_null", false);
    setParam<string>("ktype", KQuery::DAY);
}

IWithKType::IWithKType(const Indicator& ref_ind, bool fill_null, const KQuery::KType& ktype)
: IndicatorImp("WITHKTYPE"), m_ref_ind(ref_ind) {
    setParam<bool>("fill_null", fill_null);
    string nktype = ktype;
    to_upper(nktype);
    setParam<string>("ktype", nktype);
}

IWithKType::~IWithKType() {}

IndicatorImpPtr IWithKType::_clone() {
    auto p = make_shared<IWithKType>();
    p->m_ref_ind = m_ref_ind.clone();
    return p;
}

string IWithKType::str() const {
    std::ostringstream os;
    os << "Indicator{\n"
       << "  name: " << name() << "\n  size: " << size() << "\n  discard: " << discard()
       << "\n  result sets: " << getResultNumber() << "\n  params: " << getParameter()
       << "\n  formula: " << formula();
#if !HKU_USE_LOW_PRECISION
    if (m_pBuffer[0]) {
        os << "\n  values: " << *m_pBuffer[0];
    }
#endif
    os << "\n}";
    return os.str();
}

string IWithKType::formula() const {
    return fmt::format("WITHKTYPE({})", m_ref_ind.formula());
}

void IWithKType::_calculate(const Indicator& ind) {
    HKU_ASSERT(isLeaf());

    auto null_k = Null<KData>();
    auto in_k = getContext();
    auto self_k = m_ref_ind.getContext();
    if (self_k == null_k) {
        self_k = in_k;
    }
    if (in_k == null_k) {
        in_k = self_k;
    }
    HKU_IF_RETURN(self_k.empty(), void());

    KQuery::KType ktype = getParam<string>("ktype");
    const auto& query = self_k.getQuery();
    if (query.kType() != ktype) {
        KQuery new_query;
        if (query.queryType() == KQuery::DATE) {
            new_query =
              KQueryByDate(query.startDatetime(), query.endDatetime(), ktype, query.recoverType());
        } else {
            new_query = KQueryByIndex(query.start(), query.end(), ktype, query.recoverType());
        }

        auto new_self_k = self_k.getStock().getKData(new_query);
        self_k = std::move(new_self_k);
    }

    auto ref = m_ref_ind(self_k);
    auto self_dates = ref.getDatetimeList();

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
        ref = ALIGN(ref, in_k, getParam<bool>("fill_null"));
    } else if (self_dates.size() > 1) {
        // 无上下文的时间序列
        ref = ALIGN(ref, in_k, getParam<bool>("fill_null"));
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

Indicator HKU_API WITHKTYPE(const KQuery::KType& ktype, bool fill_null) {
    auto p = make_shared<IWithKType>();
    p->setParam<bool>("fill_null", fill_null);
    string nktype = ktype;
    to_upper(nktype);
    p->setParam<string>("ktype", nktype);
    return Indicator(p);
}

Indicator HKU_API WITHKTYPE(const Indicator& ind, const KQuery::KType& ktype, bool fill_null) {
    return make_shared<IWithKType>(ind, fill_null, ktype)->calculate();
}

}  // namespace hku