/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-25
 *      Author: fasiondog
 */

#include "hikyuu/utilities/thread/algorithm.h"
#include "hikyuu/indicator/crt/COST.h"
#include "IWinner.h"

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IWinner)
#endif

namespace hku {

IWinner::IWinner() : IndicatorImp("WINNER", 1) {}

IWinner::~IWinner() {}

void IWinner::_calculate(const Indicator &data) {
    size_t total = data.size();
    m_discard = data.discard();
    if (m_discard >= total) {
        m_discard = total;
        return;
    }

    // 获取输入指标的上下文
    auto context = data.getContext();
    if (context == Null<KData>()) {
        m_discard = total;
        return;
    }

    IndicatorList cost_list(101);
    value_t const *cost_data[101];

    cost_list[0] = COST(0)(context);
    m_discard = cost_list[0].discard();
    HKU_IF_RETURN(m_discard >= total, void());
    cost_data[0] = cost_list[0].data();

    parallel_for_index_void(1, 101, [&cost_data, &cost_list, &context](size_t i) {
        cost_list[i] = COST(i)(context);
        cost_data[i] = cost_list[i].data();
    });

    auto const *src = data.data();
    auto *dst = this->data();
    for (size_t i = m_discard; i < total; ++i) {
        int high_idx = 100;
        int low_idx = 0;
        while (low_idx <= high_idx) {
            value_t high = cost_data[high_idx][i];
            value_t low = cost_data[low_idx][i];
            int mid_idx = (high_idx + low_idx) / 2;
            value_t mid = cost_data[mid_idx][i];
            if (src[i] >= high) {
                dst[i] = high_idx * 0.01;
                break;
            } else if (src[i] <= low) {
                dst[i] = low_idx * 0.01;
                break;
            } else if (src[i] == mid) {
                dst[i] = mid_idx * 0.01;
                break;
            }

            if (src[i] > mid) {
                low_idx = mid_idx + 1;
                if (low_idx >= high_idx) {
                    dst[i] = low_idx * 0.01;
                    break;
                }
            } else {
                high_idx = mid_idx - 1;
                if (high_idx <= low_idx) {
                    dst[i] = low_idx * 0.01;
                    break;
                }
            }
        }
    }
}

Indicator HKU_API WINNER() {
    return Indicator(make_shared<IWinner>());
}

} /* namespace hku */
