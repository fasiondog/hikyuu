/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "ISpearman.h"

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
        HKU_ASSERT(n == 0 || n >= 2);
    }
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
    size_t total = ind.size();
    HKU_IF_RETURN(total == 0, void());

    Indicator ref = prepare(ind);

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
      [=](size_t i) {
          const auto *a = srca + i;
          const auto *b = srcb + i;
          vector<IndicatorImp::value_t> tmpa;
          vector<IndicatorImp::value_t> tmpb;
          tmpa.reserve(n);
          tmpa.reserve(n);
          for (int j = 0; j < n; j++) {
              if (!std::isnan(a[j]) && !std::isnan(b[j])) {
                  tmpa.push_back(a[j]);
                  tmpb.push_back(b[j]);
              }
          }
          int act_count = tmpa.size();
          if (act_count < 2) {
              return;
          }

          auto levela = std::vector<value_t>(n);
          auto levelb = std::vector<value_t>(n);
          auto *ptra = levela.data();
          auto *ptrb = levelb.data();

          spearmanLevel(tmpa.data(), ptra, act_count);
          spearmanLevel(tmpb.data(), ptrb, act_count);
          // Calculate the Pearson correlation coefficient on the ranks directly (tie-safe).
          // Key property: the average rank does not change the sum of the ranks, so the mean of the
          // ranks is always the prior constant (act_count+1)/2, which needs no traversal estimation
          // and eliminates the catastrophic cancellation of subtracting large numbers.
          double mean_rank = (act_count + 1) / 2.0;
          double var_r = 0.0, var_s = 0.0, cov = 0.0;
          for (int j = 0; j < act_count; j++) {
              double dev_r = ptra[j] - mean_rank;
              double dev_s = ptrb[j] - mean_rank;
              var_r += dev_r * dev_r;
              var_s += dev_s * dev_s;
              cov += dev_r * dev_s;
          }
          // On a zero variance (the factors or the returns are all equal in the cross section) it
          // returns 0.0: the semantics is "no discrimination means no predictive power". NaN is not
          // returned, because a downstream MA indicator spreads NaN like "one NaN makes all NaN"
          // (and a NaN in the first window silently outputs a low wrong value), the weighted
          // combination has no NaN guard, and NaN would spread and pollute the weights of about
          // ic_rolling_n days.
          if (var_r == 0.0 || var_s == 0.0) {
              dst[i] = 0.0;
          } else {
              double rho = cov / std::sqrt(var_r * var_s);
              // Clamp the out of range caused by the floating point error
              dst[i] = std::max(-1.0, std::min(1.0, rho));
          }
      },
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