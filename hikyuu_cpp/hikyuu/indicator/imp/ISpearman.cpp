/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "hikyuu/indicator/crt/ALIGN.h"
#include "ISpearman.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISpearman)
#endif

namespace hku {

ISpearman::ISpearman() : IndicatorImp("SPEARMAN") {
    setParam<int>("n", 0);
}

ISpearman::ISpearman(int n) : IndicatorImp("SPEARMAN") {
    setParam<int>("n", n);
}

ISpearman::ISpearman(const Indicator &ref_ind, int n)
: IndicatorImp("SPEARMAN"), m_ref_ind(ref_ind) {
    setParam<int>("n", n);
}

ISpearman::~ISpearman() {}

void ISpearman::_checkParam(const string &name) const {
    if ("n" == name) {
        int n = getParam<int>("n");
        HKU_ASSERT(n == 0 || n >= 2);
    }
}

IndicatorImpPtr ISpearman::_clone() {
    ISpearman *p = new ISpearman();
    p->m_ref_ind = m_ref_ind.clone();
    return IndicatorImpPtr(p);
}

static void spearmanLevel(const IndicatorImp::value_t *data, IndicatorImp::value_t *level,
                          size_t total) {
    std::vector<std::pair<IndicatorImp::value_t, size_t>> data_index(total);
    for (size_t i = 0; i < total; i++) {
        data_index[i].first = data[i];
        data_index[i].second = i;
    }

    std::sort(
      data_index.begin(), data_index.end(),
      std::bind(
        std::less<IndicatorImp::value_t>(),
        std::bind(&std::pair<IndicatorImp::value_t, size_t>::first, std::placeholders::_1),
        std::bind(&std::pair<IndicatorImp::value_t, size_t>::first, std::placeholders::_2)));

    size_t i = 0;
    while (i < total) {
        size_t count = 1;
        IndicatorImp::value_t score = i + 1.0;
        for (size_t j = i + 1; j < total; j++) {
            if (data_index[i].first != data_index[j].first) {
                break;
            }
            count++;
            score += j + 1;
        }
        score = score / count;
        for (size_t j = 0; j < count; j++) {
            level[data_index[i + j].second] = score;
        }
        i += count;
    }
}

void ISpearman::_calculate(const Indicator &ind) {
    auto k = getContext();
    m_ref_ind.setContext(k);
    Indicator ref = m_ref_ind;
    if (m_ref_ind.size() != ind.size()) {
        ref = ALIGN(m_ref_ind, ind);
    }

    size_t total = ind.size();
    _readyBuffer(total, 1);
    HKU_IF_RETURN(total == 0, void());

    int n = getParam<int>("n");
    if (n == 0) {
        n = total;
    }

    m_discard = std::max(ind.discard(), ref.discard());
    m_discard += n - 1;
    if (m_discard > total) {
        m_discard = total;
        return;
    }

    auto levela = std::make_unique<value_t[]>(n);
    auto levelb = std::make_unique<value_t[]>(n);
    auto *ptra = levela.get();
    auto *ptrb = levelb.get();

    // 不处理 n 不足的情况，防止只需要计算全部序列时，过于耗时
    double back = std::pow(n, 3) - n;
    vector<IndicatorImp::value_t> tmpa;
    vector<IndicatorImp::value_t> tmpb;
    tmpa.reserve(n);
    tmpa.reserve(n);

    auto *dst = this->data();
    auto const *a = ind.data() + m_discard + 1 - n;
    auto const *b = ref.data() + m_discard + 1 - n;
    for (size_t i = m_discard; i < total; ++i) {
        tmpa.clear();
        tmpb.clear();
        for (int j = 0; j < n; j++) {
            if (!std::isnan(a[j]) && !std::isnan(b[j])) {
                tmpa.push_back(a[j]);
                tmpb.push_back(b[j]);
            }
        }
        int act_count = tmpa.size();
        if (act_count < 2) {
            continue;
        }
        spearmanLevel(tmpa.data(), ptra, act_count);
        spearmanLevel(tmpb.data(), ptrb, act_count);
        value_t sum = 0.0;
        for (int j = 0; j < act_count; j++) {
            sum += std::pow(ptra[j] - ptrb[j], 2);
        }
        dst[i] = act_count == n ? 1.0 - 6.0 * sum / back
                                : 1.0 - 6.0 * sum / (std::pow(act_count, 3) - act_count);
        a++;
        b++;
    }
}

Indicator HKU_API SPEARMAN(int n) {
    return Indicator(make_shared<ISpearman>(n));
}

Indicator HKU_API SPEARMAN(const Indicator &ind1, const Indicator &ind2, int n) {
    auto p = make_shared<ISpearman>(ind2, n);
    Indicator result(p);
    return result(ind1);
}

}  // namespace hku