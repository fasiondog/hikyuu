/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-10
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * PF rebalancing cycle indicator, mainly used for verifying the PF rebalancing days and as SG
 * @param k the associated K-line data
 * @param adjust_cycle rebalancing cycle, 1 by default
 * @param adjust_mode rebalancing mode, "query" by default: day|week|month|quarter|year|query
 * @param delay_to_trading_day delay to the trading day; when the rebalancing day is a non-trading
 *                             day, it is automatically delayed to the next trading day as the
 *                             rebalancing day
 * @ingroup Indicator
 */
Indicator HKU_API CYCLE(const KData& k, int adjust_cycle = 1, const string& adjust_mode = "query",
                        bool delay_to_trading_day = true);
Indicator HKU_API CYCLE(int adjust_cycle = 1, const string& adjust_mode = "query",
                        bool delay_to_trading_day = true);

inline void registerPart_CYCLE() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "CYCLE";
    meta.params.push_back(ParamMeta{.name = "k", .type = "KData", .required = true});
    meta.params.push_back(ParamMeta{.name = "adjust_cycle", .type = "int", .default_value = "1"});
    meta.params.push_back(
      ParamMeta{.name = "adjust_mode", .type = "string", .default_value = "query"});
    meta.params.push_back(
      ParamMeta{.name = "delay_to_trading_day", .type = "bool", .default_value = "true"});
    registerIndicatorPart(
      "core.ind.cycle", meta, [](const Parameter& sc, const PartParamMap&) -> Part {
          return CYCLE(sc.get<KData>("k"), static_cast<int>(sc.get<int64_t>("adjust_cycle")),
                       sc.get<string>("adjust_mode"), sc.get<bool>("delay_to_trading_day"));
      });
}

}  // namespace hku
