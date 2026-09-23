/*
 * hikyuu.h
 *
 *  Created on: 2011-11-13
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_H_
#define HIKYUU_H_

#include "KData.h"
#include "Stock.h"
#include "StockManager.h"
#include "utilities/Parameter.h"
#include "trade_manage/build_in.h"
#include "trade_sys/all.h"
#include "strategy/Strategy.h"
#include "indicator/build_in.h"

namespace hku {

/**
 * @ingroup StockManage
 * @{
 */

/**
 * Hikyuu core initialization, the client must call it at the program entry
 * @param config_file_name config file name
 * @param ignore_preload ignore the preload settings in the config, i.e. do not load the data into
 *                        memory. Used in the cases where hikyuu is started only to get the basic
 *                        information of the database.
 * @param context the given data loading context, used to load the given stock data only when the
 *                strategy runs standalone
 */
void HKU_API hikyuu_init(const string& config_file_name, bool ignore_preload = false,
                         const StrategyContext& context = StrategyContext({"all"}));

/**
 * @brief Try to get the config parameters from the default config file to initialize
 * @param context strategy context
 * @param ignore_preload ignore the preload settings in the config, i.e. do not load the data into
 *                        memory. Used in the cases where hikyuu is started only to get the basic
 *                        information of the database.
 */
void HKU_API hikyuu_init(const StrategyContext& context = StrategyContext({"all"}),
                         bool ignore_preload = false);

/**
 * @brief Try to get the config parameters from the ini file
 * @param config_file_name ini file name
 * @param baseParam [out]
 * @param blockParam [out]
 * @param kdataParam [out]
 * @param preloadParam [out]
 * @param hkuParam [out]
 */
void HKU_API getConfigFromIni(const string& config_file_name, Parameter& baseParam,
                              Parameter& blockParam, Parameter& kdataParam, Parameter& preloadParam,
                              Parameter& hkuParam);

/** @} */

}  // namespace hku

#endif /* HIKYUU_H_ */
