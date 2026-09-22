/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/factor/FactorSet.h"

namespace hku {

/** Save the factor */
void HKU_API saveFactor(const Factor& factor, bool update_before = true);

/** Save the special factor values */
void HKU_API saveSpecialFactorValues(const Factor& factor, const Stock& stock,
                                     const DatetimeList& dates, const PriceList& values,
                                     bool replace);

bool HKU_API hasFactor(const string& name, const KQuery::KType& ktype = KQuery::DAY);

/** Get the factor */
Factor HKU_API getFactor(const string& name, const KQuery::KType& ktype = KQuery::DAY);

/** Delete the factor */
void HKU_API removeFactor(const string& name, const KQuery::KType& ktype = KQuery::DAY);

/** Get all the factors */
FactorList HKU_API getAllFactors();

/** Get all the factor sets */
FactorSetList HKU_API getAllFactorSets();

/** Update all the saved factor data to the current */
void HKU_API updateAllFactorsValues(const KQuery::KType& ktype = KQuery::DAY);

/** Save the factor set */
void HKU_API saveFactorSet(const FactorSet& set);

/** Delete the factor set */
void HKU_API removeFactorSet(const string& name, const KQuery::KType& ktype);

/** Get the factor set */
FactorSet HKU_API getFactorSet(const string& name, const KQuery::KType& ktype = KQuery::DAY);

bool isValidFactorName(const string& name);

IndicatorList getValues(const Factor& factor, const StockList& stocks, const KQuery& query,
                        bool align, bool fill_null, bool tovalue, const DatetimeList& align_dates);

vector<IndicatorList> getValues(const FactorSet& factor_set, const StockList& stocks,
                                const KQuery& query, bool align, bool fill_null, bool tovalue,
                                const DatetimeList& align_dates);

}  // namespace hku