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

IBarsLasts::IBarsLasts() : IndicatorImp("BARSLASTS", 2) {}

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
    
    // 如果条件成立次数不足N次，全部返回NaN
    if (true_positions.size() < n) {
        m_discard = total;
        return;
    }
    
    // 找到第N次条件成立的位置，从该位置开始有效
    size_t first_valid_pos = true_positions[n - 1];
    
    // 先将所有数据初始化为NaN
    for (size_t i = 0; i < total; i++) {
        dst[i] = Null<price_t>();
    }
    
    // 反向遍历，类似BARSLAST的逻辑
    size_t pos = total;
    size_t count = 0;  // 记录已经处理了多少次条件成立
    
    for (size_t i = total - 1; i >= first_valid_pos; i--) {
        if (src[i] != 0.0) {
            count++;
            size_t target_idx = true_positions.size() - count;
            size_t target_pos = true_positions[target_idx];
            size_t base_pos = true_positions[target_idx - n + 1];
            size_t target_pos_end = (target_idx + 1) >= true_positions.size()? pos : true_positions[target_idx + 1];
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

    // 最后设置discard
    m_discard = first_valid_pos;
}

Indicator HKU_API BARSLASTS(int n) {
    auto p = make_shared<IBarsLasts>();
    p->setParam<int>("n", n);
    return Indicator(p);
}

Indicator HKU_API BARSLASTS(const Indicator& n_ind) {
    auto p = make_shared<IBarsLasts>();
    p->setParam<Indicator>("n", n_ind);
    return Indicator(p);
}

} /* namespace hku */
