/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include <climits>
#include "ISpearman.h"
#include "hikyuu/indicator/utils.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::ISpearman)
#endif

namespace hku {

ISpearman::ISpearman() : Indicator2InImp("SPEARMAN") {
    setParam<int>("n", 0);
}

ISpearman::ISpearman(const Indicator &ref_ind, int n, bool fill_null)
: Indicator2InImp("SPEARMAN", ref_ind, fill_null, 1) {
    setParam<int>("n", n);
}

ISpearman::~ISpearman() {}

void ISpearman::_checkParam(const string &name) const {
    if ("n" == name) {
        int n = getParam<int>("n");
        HKU_ASSERT(n == 0 || (n >= 2 && n < INT_MAX));
    }
}

void ISpearman::_calculate(const Indicator &ind) {
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    Indicator ref = prepare(ind);

    int n = getParam<int>("n");
    size_t startPos = std::max(ind.discard(), ref.discard());

    // n == 0: expanding-window Spearman, dst[i] is the rank corr of the prefix [startPos, i]
    if (0 == n) {
        m_discard = startPos + 1;
        if (m_discard >= total) {
            m_discard = total;
            return;
        }
        auto *dst = this->data();
        auto const *srca_full = ind.data();
        auto const *srcb_full = ref.data();
        global_parallel_for_index_void(
          m_discard, total,
          [=](size_t i) {
              size_t win = i - startPos + 1;
              dst[i] =
                detail::cross_section_spearman(srca_full + startPos, srcb_full + startPos, win);
          },
          100);
        return;
    }

    m_discard = startPos + n - 1;
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    _increment_calculate(ind, m_discard);
}

bool ISpearman::supportIncrementCalculate() const {
    return getParam<int>("n") > 0;
}

size_t ISpearman::min_increment_start() const {
    return getParam<int>("n");
}

void ISpearman::_increment_calculate(const Indicator &ind, size_t start_pos) {
    size_t total = ind.size();
    Indicator ref = prepare(ind);

    int n = getParam<int>("n");
    // Consistent with _calculate: n=0 means the whole window. _calculate passes only the
    // normalized n to m_discard and then delegates to this function; this function calls getParam
    // again and would read the original 0, so it must be converted once more.
    if (n == 0) {
        n = total;
    }
    auto *dst = this->data();
    auto const *srca = ind.data() + 1 - n;
    auto const *srcb = ref.data() + 1 - n;
    global_parallel_for_index_void(
      start_pos, total,
      [=](size_t i) { dst[i] = detail::cross_section_spearman(srca + i, srcb + i, (size_t)n); },
      100);
}

Indicator HKU_API SPEARMAN(const Indicator &ref_ind, int n, bool fill_null) {
    return Indicator(make_shared<ISpearman>(ref_ind, n, fill_null));
}

Indicator HKU_API SPEARMAN(const Indicator &ind, const Indicator &ref_ind, int n, bool fill_null) {
    auto p = make_shared<ISpearman>(ref_ind, n, fill_null);
    Indicator result(p);
    return result(ind);
}

}  // namespace hku
