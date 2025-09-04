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
 * @brief 将指标数据转换到指定周期
 * @param ktype 指定周期
 * @param fill_null 扩展时填充null数据，否则为使用最近值填充
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
 * @brief 计算指标值在指定板块中的排名
 * @param block 指定板块
 * @param ref_ind 指标
 * @param mode 排名模式，0-降序排名(指标值最高值排名为1), 1-升序排名(指标值越大排名值越大),
 * 2-降序排名百分比, 3-升序排名百分比
 * @param fill_null 是否填充null数据
 * @param market 指定市场（对齐日期）
 * @return Indicator
 */
Indicator HKU_API RANK(const Block& block, const Indicator& ref_ind, int mode = 0,
                       bool fill_null = true, const string& market = "SH");
Indicator HKU_API RANK(const Block& block, int mode = 0, bool fill_null = true,
                       const string& market = "SH");

/**
 * @brief 聚合其他K线周期统计值，聚合计算指定K线类型数据均值到以上下文中的K线周期
 * @param ktype 指定K线周期
 * @param fill_null 是否填充null数据
 * @param unit 聚合周期单位，默认为1。按上下文K线 unit 个周期计算
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

Indicator HKU_API AGG_STD(const Indicator& ind, const KQuery::KType& ktype = KQuery::MIN,
                          bool fill_null = false, int unit = 1, int ddof = 1);
Indicator HKU_API AGG_VAR(const Indicator& ind, const KQuery::KType& ktype = KQuery::MIN,
                          bool fill_null = false, int unit = 1, int ddof = 1);
Indicator HKU_API AGG_QUANTILE(const Indicator& ind, const KQuery::KType& ktype = KQuery::MIN,
                               bool fill_null = false, int unit = 1, double quantile = 0.75);

/** @} */
}  // namespace hku