/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"

namespace hku {

/**
 * @ingroup Indicator
 * @{
 */

/**
 * @brief Fix the start index of the query range used in the indicator calculation
 * @param start_index the start index position; when it is negative, the calculation starts from the
 *                    index time points before the current latest one
 * @return Indicator
 * @note For some indicators that change with the time start point (such as AD), the fixed start
 *       index ensures that the calculation starts from the first data of the stock.
 *       Example: FIXED_START_INDEX(AD())(getKData("sz000001", Query(-500)))
 */
Indicator HKU_API FIXED_START_INDEX(int start_index = 0);

/**
 * @brief Fix the start index of the query range used in the indicator calculation
 * @param ind the input indicator
 * @param start_index the start index position; when it is negative, the calculation starts from the
 *                    index time points before the current latest one
 * @return Indicator
 * @note For some indicators that change with the time start point (such as AD), the fixed start
 *       index ensures that the calculation starts from the first data of the stock.
 *       Example: FIXED_START_INDEX(AD(), 0)(getKData("sz000001", Query(-500)))
 */
Indicator HKU_API FIXED_START_INDEX(const Indicator& ind, int start_index = 0);

/**
 * @brief Fix the start date of the query range used in the indicator calculation
 * @param start_date start date
 * @return Indicator
 * @note For some indicators that change with the time start point (such as AD), the fixed start
 *       date ensures that the calculation starts from the given date.
 *       Example: FIXED_START_DATE(AD(), Datetime("2020-01-01"))(getKData("sz000001", Query(-500)))
 */
Indicator HKU_API FIXED_START_DATE(const Datetime& start_date = Datetime::min());

/**
 * @brief Fix the start date of the query range used in the indicator calculation
 * @param ind the input indicator
 * @param start_date start date
 * @return Indicator
 * @note For some indicators that change with the time start point (such as AD), the fixed start
 *       date ensures that the calculation starts from the given date.
 *       Example: FIXED_START_DATE(AD(), Datetime("2020-01-01"))(getKData("sz000001", Query(-500)))
 */
Indicator HKU_API FIXED_START_DATE(const Indicator& ind,
                                   const Datetime& start_date = Datetime::min());

/**
 * @brief Convert the indicator data to the given period
 * @param ktype the given period
 * @param fill_null fill the null data when extending, otherwise the latest value is used to fill
 * @return Indicator
 */
Indicator HKU_API WITHKTYPE(const KQuery::KType& ktype, bool fill_null = false);

Indicator HKU_API WITHKTYPE(const Indicator& ind, const KQuery::KType& ktype,
                            bool fill_null = false);

inline Indicator WITHDAY(bool fill_null = false) {
    return WITHKTYPE(KQuery::DAY, fill_null);
}

inline Indicator WITHDAY(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::DAY, fill_null);
}

inline Indicator WITHWEEK(bool fill_null = false) {
    return WITHKTYPE(KQuery::WEEK, fill_null);
}

inline Indicator WITHWEEK(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::WEEK, fill_null);
}

inline Indicator WITHMONTH(bool fill_null = false) {
    return WITHKTYPE(KQuery::MONTH, fill_null);
}

inline Indicator WITHMONTH(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::MONTH, fill_null);
}

inline Indicator WITHQUARTER(bool fill_null = false) {
    return WITHKTYPE(KQuery::QUARTER, fill_null);
}

inline Indicator WITHQUARTER(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::QUARTER, fill_null);
}

inline Indicator WITHHALFYEAR(bool fill_null = false) {
    return WITHKTYPE(KQuery::HALFYEAR, fill_null);
}

inline Indicator WITHHALFYEAR(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::HALFYEAR, fill_null);
}

inline Indicator WITHYEAR(bool fill_null = false) {
    return WITHKTYPE(KQuery::YEAR, fill_null);
}

inline Indicator WITHYEAR(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::YEAR, fill_null);
}

inline Indicator WITHMIN(bool fill_null = false) {
    return WITHKTYPE(KQuery::MIN, fill_null);
}

inline Indicator WITHMIN(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::MIN, fill_null);
}

inline Indicator WITHMIN5(bool fill_null = false) {
    return WITHKTYPE(KQuery::MIN5, fill_null);
}

inline Indicator WITHMIN5(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::MIN5, fill_null);
}

inline Indicator WITHMIN15(bool fill_null = false) {
    return WITHKTYPE(KQuery::MIN15, fill_null);
}

inline Indicator WITHMIN15(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::MIN15, fill_null);
}

inline Indicator WITHMIN30(bool fill_null = false) {
    return WITHKTYPE(KQuery::MIN30, fill_null);
}

inline Indicator WITHMIN30(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::MIN30, fill_null);
}

inline Indicator WITHMIN60(bool fill_null = false) {
    return WITHKTYPE(KQuery::MIN60, fill_null);
}

inline Indicator WITHMIN60(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::MIN60, fill_null);
}

inline Indicator WITHHOUR(bool fill_null = false) {
    return WITHKTYPE(KQuery::MIN60, fill_null);
}

inline Indicator WITHHOUR(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::MIN60, fill_null);
}

inline Indicator WITHHOUR2(bool fill_null = false) {
    return WITHKTYPE(KQuery::HOUR2, fill_null);
}

inline Indicator WITHHOUR2(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::HOUR2, fill_null);
}

inline Indicator WITHHOUR4(bool fill_null = false) {
    return WITHKTYPE(KQuery::HOUR4, fill_null);
}

inline Indicator WITHHOUR4(const Indicator& ind, bool fill_null = false) {
    return WITHKTYPE(ind, KQuery::HOUR4, fill_null);
}

/**
 * @brief Calculate the rank of the indicator values within the given block
 * @param block the given block
 * @param ref_ind indicator
 * @param mode rank mode: 0-descending rank (the highest indicator value has the rank 1),
 *             1-ascending rank (the larger the indicator value, the larger the rank value),
 *             2-descending rank percentage, 3-ascending rank percentage
 * @param fill_null whether to fill the null data
 * @param market the given market (align the dates)
 * @return Indicator
 */
Indicator HKU_API RANK(const Block& block, const Indicator& ref_ind, int mode = 0,
                       bool fill_null = true, const string& market = "SH");
Indicator HKU_API RANK(const Block& block, int mode = 0, bool fill_null = true,
                       const string& market = "SH");

/**
 * @brief Aggregate the statistics of the other K-line periods; aggregate the mean of the data of
 * the given K-line type into the K-line period of the context
 * @param ktype the given K-line period
 * @param fill_null whether to fill the null data
 * @param unit aggregation period unit, 1 by default. It is calculated by unit periods of the
 *             context K-line
 * @return Indicator
 */
#define AGG_FUNC_DEFINE(agg_name)                                                              \
    Indicator HKU_API agg_name(const Indicator& ind, const KQuery::KType& ktype = KQuery::MIN, \
                               bool fill_null = false, int unit = 1);

#define AGG_FUNC_IMP(agg_name)                                                                   \
    Indicator HKU_API agg_name(const Indicator& ind, const KQuery::KType& ktype, bool fill_null, \
                               int unit) {                                                       \
        Parameter params;                                                                        \
        params.set<string>("ktype", ktype);                                                      \
        params.set<bool>("fill_null", fill_null);                                                \
        params.set<int>("unit", unit);                                                           \
        return getExtIndicator(#agg_name, ind, params);                                          \
    }

AGG_FUNC_DEFINE(AGG_MEAN)
AGG_FUNC_DEFINE(AGG_COUNT)
AGG_FUNC_DEFINE(AGG_SUM)
AGG_FUNC_DEFINE(AGG_MAX)
AGG_FUNC_DEFINE(AGG_MIN)
AGG_FUNC_DEFINE(AGG_MAD)
AGG_FUNC_DEFINE(AGG_MEDIAN)
AGG_FUNC_DEFINE(AGG_PROD)

/**
 * @brief Time sampling aggregation indicator
 *
 * Sample the indicator data at the given time point. If an exactly matching time cannot be found,
 * the valid data closest to and before the target time is selected.
 *
 * @param ind the input indicator
 * @param ktype the given K-line period
 * @param time the given sampling time in the format HH:MM, "9:35" by default
 * @param fill_null whether to fill the null data
 * @param unit aggregation period unit
 * @return Indicator
 */
Indicator HKU_API AGG_SAMPLE(const Indicator& ind, const string& time = "9:35",
                             const KQuery::KType& ktype = KQuery::MIN, bool fill_null = false,
                             int unit = 1);

/**
 * @brief Time range maximum aggregation indicator
 *
 * Count the maximum of the indicator data within the given time range [start_time, last_time],
 * including start_time and last_time themselves.
 *
 * @param ind the input indicator
 * @param start_time the start time of the time range in the format HH:MM, "9:30" by default
 * @param last_time the end time of the time range in the format HH:MM, "10:00" by default
 * @param ktype the given K-line period
 * @param fill_null whether to fill the null data
 * @param unit aggregation period unit
 * @return Indicator
 */
Indicator HKU_API AGG_SAMPLE_MAX(const Indicator& ind, const string& start_time = "9:30",
                                 const string& last_time = "10:00",
                                 const KQuery::KType& ktype = KQuery::MIN, bool fill_null = false,
                                 int unit = 1);

/**
 * @brief Time range minimum aggregation indicator
 *
 * Count the minimum of the indicator data within the given time range [start_time, last_time],
 * including start_time and last_time themselves.
 *
 * @param ind the input indicator
 * @param start_time the start time of the time range in the format HH:MM, "9:30" by default
 * @param last_time the end time of the time range in the format HH:MM, "10:00" by default
 * @param ktype the given K-line period
 * @param fill_null whether to fill the null data
 * @param unit aggregation period unit
 * @return Indicator
 */
Indicator HKU_API AGG_SAMPLE_MIN(const Indicator& ind, const string& start_time = "9:30",
                                 const string& last_time = "10:00",
                                 const KQuery::KType& ktype = KQuery::MIN, bool fill_null = false,
                                 int unit = 1);

/**
 * @brief Time range average aggregation indicator
 *
 * Count the average of the indicator data within the given time range [start_time, last_time],
 * including start_time and last_time themselves.
 *
 * @param ind the input indicator
 * @param start_time the start time of the time range in the format HH:MM, "9:30" by default
 * @param last_time the end time of the time range in the format HH:MM, "10:00" by default
 * @param ktype the given K-line period
 * @param fill_null whether to fill the null data
 * @param unit aggregation period unit
 * @return Indicator
 */
Indicator HKU_API AGG_SAMPLE_MEAN(const Indicator& ind, const string& start_time = "9:30",
                                  const string& last_time = "10:00",
                                  const KQuery::KType& ktype = KQuery::MIN, bool fill_null = false,
                                  int unit = 1);

Indicator HKU_API AGG_STD(const Indicator& ind, const KQuery::KType& ktype = KQuery::MIN,
                          bool fill_null = false, int unit = 1, int ddof = 1);
Indicator HKU_API AGG_VAR(const Indicator& ind, const KQuery::KType& ktype = KQuery::MIN,
                          bool fill_null = false, int unit = 1, int ddof = 1);
Indicator HKU_API AGG_QUANTILE(const Indicator& ind, const KQuery::KType& ktype = KQuery::MIN,
                               bool fill_null = false, int unit = 1, double quantile = 0.75);

Indicator HKU_API AGG_VWAP(const KQuery::KType& ktype = KQuery::MIN, bool fill_null = false,
                           int unit = 1);

inline Indicator AGG_VWAP(const KData& k, const KQuery::KType& ktype = KQuery::MIN,
                          bool fill_null = false, int unit = 1) {
    return AGG_VWAP(ktype, fill_null, unit)(k);
}

using agg_func_t = std::function<double(const DatetimeList& src_ds, const Indicator::value_t* src,
                                        size_t group_start, size_t group_last)>;
Indicator HKU_API AGG_FUNC(const Indicator& ind, agg_func_t agg_func,
                           const KQuery::KType& ktype = KQuery::MIN, bool fill_null = false,
                           int unit = 1);

/**
 * @brief Group by period
 * @param ktype the given K-line period
 * @param unit grouping period unit, 1 by default. It is calculated by unit periods of the ktype
 *             parameter
 * @return Indicator
 */
#define GROUP_FUNC_DEFINE(group_name)                                                            \
    Indicator HKU_API group_name(const Indicator& ind, const KQuery::KType& ktype = KQuery::DAY, \
                                 int unit = 1);

#define GROUP_FUNC_IMP(group_name)                                                             \
    Indicator HKU_API group_name(const Indicator& ind, const KQuery::KType& ktype, int unit) { \
        Parameter params;                                                                      \
        params.set<string>("ktype", ktype);                                                    \
        params.set<int>("unit", unit);                                                         \
        return getExtIndicator(#group_name, ind, params);                                      \
    }

GROUP_FUNC_DEFINE(GROUP_COUNT)
GROUP_FUNC_DEFINE(GROUP_SUM)
GROUP_FUNC_DEFINE(GROUP_MEAN)
GROUP_FUNC_DEFINE(GROUP_PROD)
GROUP_FUNC_DEFINE(GROUP_MIN)
GROUP_FUNC_DEFINE(GROUP_MAX)

using group_func_t =
  std::function<void(Indicator::value_t* dst, const DatetimeList& src_ds,
                     const Indicator::value_t* src, size_t group_start, size_t group_last)>;
Indicator HKU_API GROUP_FUNC(const Indicator& ind, group_func_t group_func,
                             const KQuery::KType& ktype = KQuery::DAY, int unit = 1);

/** @} */
}  // namespace hku