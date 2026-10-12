/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-13
 *      Author: fasiondog
 */

#pragma once

#include "../Indicator.h"

namespace hku {

/**
 * Walk the ex-rights/ex-dividend records of the context stock and fill every bar through the
 * callback. The shared skeleton of HSL / LIUTONGPAN / ZONGGUBEN / COST.
 *
 * @param k the context KData, not empty
 * @param start_pos the first bar to fill (0 for a full calculation)
 * @param get_count extracts the share count (freeCount / totalCount) from a StockWeight
 * @param fill writes one bar: fill(pos, krecord, count of the preceding valid record)
 * @return false when there is no record whose date is not later than the last bar, leaving the
 *         buffer untouched (all null)
 */
template <typename CountGetter, typename BarFiller>
inline bool fill_by_weight_series(const KData& k, size_t start_pos, const CountGetter& get_count,
                                  const BarFiller& fill) {
    const KRecord* kdata = k.data();
    size_t total = k.size();
    Datetime lastdate = kdata[total - 1].datetime.startOfDay();
    StockWeightList sw_list = k.getStock().getWeight(Datetime::min(), lastdate + Days(1));
    HKU_IF_RETURN(sw_list.empty(), false);

    // Find the first record whose share count is not 0
    price_t pre_count = 0.0;
    Datetime pre_sw_date;
    auto sw_iter = sw_list.begin();
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        if (get_count(*sw_iter) > 0) {
            pre_count = get_count(*sw_iter);
            pre_sw_date = sw_iter->datetime();
            break;
        }
    }
    // Return when there is no record with a positive share count, or when its date is later than
    // the last K-line date
    HKU_IF_RETURN(sw_iter == sw_list.end() || pre_sw_date > lastdate, false);

    size_t pos = start_pos;
    for (; sw_iter != sw_list.end(); ++sw_iter) {
        price_t count = get_count(*sw_iter);
        Datetime cur_sw_date = sw_iter->datetime();
        if (count <= 0.0) {
            continue;  // Ignore the ex-rights/ex-dividend record whose outstanding shares are 0
        }

        while (pos < total && kdata[pos].datetime < cur_sw_date) {
            if (kdata[pos].datetime >= pre_sw_date) {
                fill(pos, kdata[pos], pre_count);
            }
            pos++;
        }

        pre_count = count;
        pre_sw_date = cur_sw_date;
        HKU_IF_RETURN(pos >= total, true);
    }

    for (; pos < total; pos++) {
        fill(pos, kdata[pos], pre_count);
    }
    return true;
}

}  // namespace hku
