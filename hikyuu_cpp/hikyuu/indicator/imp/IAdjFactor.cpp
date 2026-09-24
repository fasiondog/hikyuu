/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-05-17
 *      Author: fasiondog
 */

#include "IAdjFactor.h"
#include <algorithm>
#include <vector>

#if HKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hku::IAdjFactor)
#endif

namespace hku {

IAdjFactor::IAdjFactor() : IndicatorImp("ADJ_FACTOR", 1) {
    m_need_context = true;
}

IAdjFactor::~IAdjFactor() {}

// Calculate the daily-line adjustment factor (the listing date is not strict, the calculation
// starts from the data contained in in_kdata). Note that as an internal function it returns an
// empty result directly when there is no ex-rights/ex-dividend data.
// base_factor: the base factor value used for the incremental calculation (1.0 by default,
// which means calculating from the beginning). Refer to the implementation of
// KDataPrivatedBufferImp::_recoverEqualBackward().
static vector<std::pair<Datetime, Indicator::value_t>> cum_adj_factor(const KData& in_kdata,
                                                                      price_t base_factor = 1.0) {
    // Get the K-line data of the corresponding daily-line range
    KData kdata = in_kdata.getKData(KQuery::DAY);

    size_t total = kdata.size();
    vector<std::pair<Datetime, Indicator::value_t>> result;
    result.reserve(total);
    HKU_IF_RETURN(total == 0, result);

    auto* krecords = kdata.data();

    // Get all the ex-rights/ex-dividend data (already sorted by date)
    Datetime start_date = krecords[0].datetime;
    Datetime end_date = krecords[total - 1].datetime + Days(1);
    StockWeightList sw_list = kdata.getStock().getWeight(start_date, end_date);

    // if (sw_list.empty()) {
    //     // There is no ex-rights/ex-dividend data, return an empty result
    //     return result;
    // }

    // Initialize all the factors to the base factor
    vector<price_t> factors(total, base_factor);

    StockWeightList::const_reverse_iterator weightIter = sw_list.rbegin();
    size_t pre_pos = total - 1;

    for (; weightIter != sw_list.rend(); ++weightIter) {
        // Find the position of the ex-rights date in the K-lines
        size_t i = pre_pos;
        while (i > 0 && krecords[i].datetime > weightIter->datetime()) {
            i--;
        }

        pre_pos = i;  // The ex-rights date position

        // Get the close price of the record date (the day before the ex-rights date)
        if (pre_pos == 0) {
            continue;  // There is no data of the previous day, it cannot be calculated
        }

        price_t closePrice = krecords[pre_pos - 1].closePrice;
        if (closePrice <= 0.0) {
            continue;
        }

        price_t denominator = 0.0, temp = closePrice;
        if (weightIter->suogu() != 0.0) {
            denominator = weightIter->suogu();
        } else {
            // The change ratio of the outstanding shares = 0.1 * (bonus shares + rights shares +
            // capitalized shares)
            price_t change = 0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                                    weightIter->increasement());
            denominator = 1.0 + change;
            temp = closePrice + weightIter->priceForSell() * change - 0.1 * weightIter->bonus();
        }

        if (temp == 0.0 || denominator == 0.0) {
            continue;
        }

        // Calculate the backward adjustment coefficient
        price_t k = (denominator * closePrice) / temp;

        // Multiply all the factors from the ex-rights date to the latest date by k
        for (size_t j = pre_pos; j < total; ++j) {
            factors[j] *= k;
        }
    }

    // Build the result
    for (size_t i = 0; i < total; ++i) {
        result.emplace_back(krecords[i].datetime, factors[i]);
    }

    return result;
}

void IAdjFactor::_calculate(const Indicator& ind) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0, void());

    _readyBuffer(total, 1);
    m_discard = 0;

    // Reuse the incremental calculation method; start_pos = 0 means calculating from the beginning
    _increment_calculate(ind, 0);
}

bool IAdjFactor::supportIncrementCalculate() const {
    return true;
}

void IAdjFactor::_increment_calculate(const Indicator& ind, size_t start_pos) {
    HKU_WARN_IF(!isLeaf() && !ind.empty(),
                "The input is ignored because {} depends on the context!", m_name);

    const KData& k = getContext();
    size_t total = k.size();
    HKU_IF_RETURN(total == 0 || start_pos >= total, void());

    auto* dst = data();
    auto* kdata = k.data();

    // Key: the adjustment factor must be calculated based on the daily line
    // Optimization strategy: use the already calculated base factor and calculate only the
    // ex-rights/ex-dividend influence of the newly added part

    // 1. Determine the base factor and the start date
    price_t base_factor = 1.0;
    Datetime calc_start_date;

    if (start_pos > 0) {
        // Use the factor value of the previous position as the base
        base_factor = dst[start_pos - 1];
        calc_start_date = kdata[start_pos - 1].datetime.startOfDay();
    } else if (!m_old_context.empty()) {
        // The first incremental calculation with an old context: use the start date of the old
        // context
        base_factor = 1.0;  // Calculate from the beginning
        calc_start_date = m_old_context[0].datetime.startOfDay();
    } else {
        // The real first calculation, starting from the first K-line
        calc_start_date = kdata[0].datetime.startOfDay();
    }

    Datetime calc_end_date = kdata[total - 1].datetime + k.getQuery().kTypeInSeconds();

    // 2. Get the K-line data corresponding to the incremental part (starting from calc_start_date)
    KData inc_kdata = k.getStock().getKData(KQueryByDate(calc_start_date, calc_end_date));

    if (inc_kdata.empty()) {
        // There is no data, the whole incremental part uses the base factor
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = base_factor;
        }
        return;
    }

    // 3. Calculate the daily-line adjustment factor of the incremental part (the base factor is
    // passed in)
    auto daily_factors = cum_adj_factor(inc_kdata, base_factor);
    if (k.getQuery().kType() == KQuery::DAY) {
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = daily_factors[i - start_pos].second;
        }
        return;
    }

    if (daily_factors.empty()) {
        // There is no ex-rights/ex-dividend data, the whole incremental part uses the base factor
        for (size_t i = start_pos; i < total; ++i) {
            dst[i] = base_factor;
        }
        return;
    }

    // 4. Align the daily-line adjustment factor to the current K-line period, starting from
    // start_pos
    size_t d_idx = 0;
    Datetime start_date = kdata[start_pos].datetime.startOfDay();

    // Find the index of the daily-line factor matching the date of start_pos
    while (d_idx < daily_factors.size() && daily_factors[d_idx].first < start_date) {
        ++d_idx;
    }

    // 5. Fill the incremental part starting from start_pos
    price_t cumulative_factor = base_factor;

    for (size_t i = start_pos; i < total; ++i) {
        const Datetime& k_date = kdata[i].datetime.startOfDay();

        // Process all the daily-line factors earlier than or equal to the current K-line date
        while (d_idx < daily_factors.size() && daily_factors[d_idx].first <= k_date) {
            cumulative_factor = daily_factors[d_idx].second;
            ++d_idx;
        }

        // Set the adjustment factor of the current K-line
        dst[i] = cumulative_factor;
    }
}

Indicator HKU_API ADJ_FACTOR() {
    return Indicator(make_shared<IAdjFactor>());
}

Indicator HKU_API ADJ_FACTOR(const KData& k) {
    auto p = make_shared<IAdjFactor>();
    p->setContext(k);
    return Indicator(p);
}

} /* namespace hku */