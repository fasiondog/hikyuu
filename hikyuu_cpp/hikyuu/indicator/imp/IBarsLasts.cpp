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
    size_t total = ind.size();
    size_t ind_discard = ind.discard();
    if (ind_discard >= total) {
        m_discard = total;
        return;
    }

    // Get the parameter N
    int n = getParam<int>("n");

    // Parameter validation: if n <= 0, return a sequence of all NaN
    if (n <= 0) {
        m_discard = total;
        return;
    }

    auto const* src = ind.data();
    auto* dst = this->data();

    // Special case: there is only one valid data point
    if (total == ind_discard + 1) {
        if (src[ind_discard] != 0.0 && n == 1) {
            dst[ind_discard] = 0.0;
            m_discard = ind_discard;
        } else {
            m_discard = total;
        }
        return;
    }

    // Record the positions where the condition holds
    std::vector<size_t> true_positions;
    for (size_t i = ind_discard; i < total; i++) {
        if (src[i] != 0.0) {
            true_positions.push_back(i);
        }
    }

    // If the condition holds fewer than N times, return NaN for everything
    if (true_positions.size() < n) {
        m_discard = total;
        return;
    }

    // Find the position where the condition holds for the N-th time; it is valid from that position
    size_t first_valid_pos = true_positions[n - 1];

    // Traverse in reverse order, similar to the BARSLAST logic
    size_t pos = total;
    size_t count = 0;  // Records how many times the condition has held

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
    // Get the dynamic parameter n
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

    // Calculate for every position separately
    for (size_t i = m_discard; i < total; i++) {
        int n = static_cast<int>(n_data[i]);

        // Parameter validation: if n <= 0, return NaN
        if (n <= 0) {
            dst[i] = Null<price_t>();
            continue;
        }

        // Search backward from the current position for the position where the condition holds for
        // the n-th time
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

        // If the position where the condition holds for the n-th time is found, calculate the
        // distance
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
