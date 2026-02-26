/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "IMdd.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IMdd)
#endif

namespace hku {

IMdd::IMdd() : IndicatorImp("MDD", 1) {
    setParam<int>("n", 0);
}

IMdd::~IMdd() {}

void IMdd::_checkParam(const string& name) const {
    if ("n" == name) {
        HKU_ASSERT(getParam<int>("n") >= 0);
    }
}

/*
 * 滑动窗口最大回撤计算（单函数版，O(n)复杂度）
 * @param nav_series 按时间顺序排列的净值数组（必须全>0）
 * @param window_size 滑动窗口大小（必须>=1）
 * @return 每个位置对应的滑动窗口最大回撤值数组（0~1之间）
 * @throws std::invalid_argument 输入参数不合法时抛出异常
 */
void calculate_sliding_window_max_drawdown(const Indicator::value_t* nav_series, size_t total,
                                           size_t window_size, Indicator::value_t* output) {
    HKU_INFO("total: {}, window_size: {}", total, window_size);
    std::vector<Indicator::value_t> drawdown_series;  // 每个点相对于窗口内最大值的回撤
    std::deque<size_t> max_nav_queue;                 // 维护窗口内净值最大值的单调队列（存索引）
    std::deque<size_t> max_dd_queue;                  // 维护窗口内回撤最大值的单调队列（存索引）

    for (size_t i = 0; i < total; ++i) {
        Indicator::value_t current_nav = nav_series[i];

        // 步骤1：维护窗口内净值最大值的单调队列
        // 移除队尾所有<=当前净值的元素（不可能成为后续最大值）
        while (!max_nav_queue.empty() && current_nav >= nav_series[max_nav_queue.back()]) {
            max_nav_queue.pop_back();
        }
        max_nav_queue.push_back(i);
        // 移除队头超出窗口范围的元素
        while (!max_nav_queue.empty() && max_nav_queue.front() <= i - window_size) {
            max_nav_queue.pop_front();
        }

        // 步骤2：计算当前点的回撤值百分比
        double window_max_nav = nav_series[max_nav_queue.front()];
        double current_dd = (window_max_nav - current_nav) / window_max_nav * 100.0;
        drawdown_series.push_back(current_dd);

        // 步骤3：维护窗口内回撤最大值的单调队列
        while (!max_dd_queue.empty() && current_dd >= drawdown_series[max_dd_queue.back()]) {
            max_dd_queue.pop_back();
        }
        max_dd_queue.push_back(i);
        // 移除队头超出窗口范围的元素
        while (!max_dd_queue.empty() && max_dd_queue.front() <= i - window_size) {
            max_dd_queue.pop_front();
        }

        // 步骤4：记录当前窗口的最大回撤
        output[i] = drawdown_series[max_dd_queue.front()];
    }
}

void IMdd::_calculate(const Indicator& ind) {
    m_discard = ind.discard();
    size_t total = ind.size();
    HKU_IF_RETURN(m_discard >= total, void());

    auto const* src = ind.data();
    auto* dst = this->data();

    size_t n = static_cast<size_t>(getParam<int>("n"));
    if (n == 0 || n > total - m_discard) {
        n = total - m_discard;
    }

    if (n == total - m_discard) {
        value_t pre_max = src[m_discard];
        value_t min_dd = 0.0;
        for (size_t i = m_discard; i < total; i++) {
            if (src[i] > pre_max) {
                pre_max = src[i];
            }
            value_t dd =
              (src[i] >= pre_max || pre_max == 0.) ? 0.0 : (src[i] / pre_max - 1.0) * 100.0;
            if (dd < min_dd) {
                min_dd = dd;
            }
            dst[i] = min_dd;
        }
        return;
    }

    bool has_non_positive = false;
    for (size_t i = m_discard; i < total; i++) {
        if (std::isnan(src[i]) || src[i] <= 0.0) {
            has_non_positive = true;
            break;
        }
    }

    if (!has_non_positive) {
        calculate_sliding_window_max_drawdown(src + m_discard, total - m_discard, n,
                                              dst + m_discard);
        return;
    }

    std::vector<size_t> index_list;
    std::vector<value_t> buf;
    buf.reserve(total - m_discard);
    for (size_t i = m_discard; i < total; i++) {
        if (src[i] > 0.0) {
            buf.push_back(src[i]);
            index_list.push_back(i);
        }
    }

    vector<value_t> tmp_result(buf.size());
    calculate_sliding_window_max_drawdown(buf.data(), buf.size(), n, tmp_result.data());
    for (size_t i = 0; i < tmp_result.size(); i++) {
        dst[index_list[i]] = tmp_result[i];
    }
}

bool IMdd::supportIncrementCalculate() const {
    return getParam<int>("n") > 0;
}

size_t IMdd::min_increment_start() const {
    return getParam<int>("n") - 1;
}

void IMdd::_increment_calculate(const Indicator& ind, size_t start_pos) {
    size_t total = ind.size();
    m_discard = ind.discard();
    size_t n = static_cast<size_t>(getParam<int>("n"));
    auto const* src = ind.data();
    auto* dst = this->data();

    price_t max = src[start_pos];
    size_t pre_pos = start_pos + 1 - n;
    for (size_t k = pre_pos + 1; k <= start_pos; k++) {
        if (src[k] > max) {
            max = src[k];
            pre_pos = k;
        }
    }

    for (size_t i = start_pos; i < total; i++) {
        size_t j = i + 1 - n;
        if (pre_pos < j) {
            pre_pos = j;
            max = src[j];
            for (size_t k = pre_pos + 1; k <= i; k++) {
                if (src[k] > max) {
                    max = src[k];
                    pre_pos = k;
                }
            }
        } else if (src[i] > max) {
            max = src[i];
            pre_pos = i;
        }
        dst[i] = (src[i] >= max || max == 0.) ? 0.0 : (src[i] / max - 1.0);
    }
}

Indicator HKU_API MDD(int n) {
    IndicatorImpPtr p = make_shared<IMdd>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

}  // namespace hku