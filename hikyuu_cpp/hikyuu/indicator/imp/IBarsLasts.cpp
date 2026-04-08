/*
 * IBarsLasts.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: hikyuu
 */

#include "IBarsLasts.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IBarsLasts)
#endif

namespace hku {

IBarsLasts::IBarsLasts() : IndicatorImp("BARSLASTS", 1) {
    setParam<int>("n", 0);
}

IBarsLasts::~IBarsLasts() {}

void IBarsLasts::_calculate(const Indicator& ind) {
    // 获取参数N
    size_t n = getParam<int>("n");

    // 参数验证：如果n <= 0，返回全NaN序列
    if (n <= 0) {
        m_discard = ind.size();
        return;
    }

    size_t total = ind.size();
    size_t ind_discard = ind.discard();
    if (ind_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    // 特殊情况：只有一个有效数据点
    if (total == ind_discard + 1) {
        if (src[ind_discard] != 0.0 && n == 1) {
            dst[ind_discard] = 0.0;
            m_discard = ind_discard;
        } else {
            m_discard = total;
        }
        return;
    }

    // 记录所有条件成立的位置
    std::vector<size_t> true_positions;
    for (size_t i = ind_discard; i < total; i++) {
        if (src[i] != 0.0) {
            true_positions.push_back(i);
        }
    }

    // 如果条件成立次数不足 N 次，全部返回 NaN
    if (true_positions.size() < n) {
        m_discard = total;
        return;
    }

    // 找到第 N 次条件成立的位置，从该位置开始有效
    size_t first_valid_pos = true_positions[n - 1];

    // 反向遍历，类似 BARSLAST 的逻辑
    size_t pos = total;
    size_t count = 0;  // 记录已经处理了多少次条件成立

    for (size_t i = total - 1; i >= first_valid_pos; i--) {
        if (src[i] != 0.0) {
            count++;
            size_t target_idx = true_positions.size() - count;

            size_t target_pos = true_positions[target_idx];
            size_t base_pos = true_positions[target_idx + 1 - n];
            size_t target_pos_end =
              (target_idx + 1) >= true_positions.size() ? pos : true_positions[target_idx + 1];
            size_t pos_diff = target_pos - base_pos;
            for (size_t j = target_pos; j < target_pos_end; j++) {
                dst[j] = j + pos_diff - target_pos;
            }
            pos = i;
        }

        if (i == first_valid_pos) {
            break;
        }
    }

    updateDiscard();
}

void IBarsLasts::_dyn_calculate(const Indicator& ind) {
    // 获取动态参数 n
    Indicator ind_param(getIndParamImp("n"));
    HKU_CHECK(ind_param.size() == ind.size(), "ind_param->size()={}, ind.size()={}!",
              ind_param.size(), ind.size());

    size_t total = ind.size();
    size_t ind_discard = ind.discard();
    m_discard = std::max(ind_discard, ind_param.discard());

    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();
    auto const* n_data = ind_param.data();

    // 对每个位置单独计算
    for (size_t i = m_discard; i < total; i++) {
        int n = static_cast<int>(n_data[i]);

        // 参数验证：如果n <= 0，返回NaN
        if (n <= 0) {
            dst[i] = Null<price_t>();
            continue;
        }

        // 从当前位置向前查找第 n 次条件成立的位置
        int count = 0;
        size_t target_pos = Null<size_t>();

        for (size_t j = i; j >= ind_discard; j--) {
            if (src[j] != 0.0) {
                count++;
                if (count == n) {
                    target_pos = j;
                    break;
                }
            }
            if (j == ind_discard) {
                break;
            }
        }

        // 如果找到了第 n 次条件成立的位置，计算距离
        if (target_pos != Null<size_t>()) {
            dst[i] = static_cast<price_t>(i - target_pos);
        } else {
            dst[i] = Null<price_t>();
        }
    }

    updateDiscard();
}

Indicator HKU_API BARSLASTS(int n) {
    auto p = make_shared<IBarsLasts>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API BARSLASTS(const IndParam& n) {
    auto p = make_shared<IBarsLasts>();
    p->setIndParam("n", n);
    return Indicator(p);
}

} /* namespace hku */
