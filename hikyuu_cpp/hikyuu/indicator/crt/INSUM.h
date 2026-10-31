/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/part/PartRegister.h"

namespace hku {

/**
 * Return the calculated value of the corresponding output of this indicator for every member of the
 * block according to the calculation type.
 * @note Note: when INSUM uses mode 4/5 it is equivalent to the RANK function, but it is not
 *       suitable for use in MF; when it is used in MF the calculation amount is of the N x N
 *       level, and the calculation is slow. If it is expected to be used in MF, it is recommended
 *       to use the RANK [donation user] indicator directly.
 * @param block the given block
 * @param query the given range
 * @param ind the given indicator
 * @param mode calculation type: 0-accumulation, 1-average, 2-maximum, 3-minimum, 4-descending
 *             rank (the highest indicator value has the rank 1),
 *             5-ascending rank (the lowest indicator value has the rank 1),
 * @param fill_null fill the missing data with nan when the dates are aligned.
 * @return Indicator
 */
Indicator HKU_API INSUM(const Block& block, const KQuery& query, const Indicator& ind, int mode,
                        bool fill_null = true);

Indicator HKU_API INSUM(const Block& block, const Indicator& ind, int mode, bool fill_null = true);

Indicator HKU_API INSUM(const Block& block, int mode, bool fill_null);

inline void registerPart_INSUM() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "INSUM";
    meta.params.push_back(ParamMeta{.name = "block", .type = "Block", .required = true});
    meta.params.push_back(ParamMeta{.name = "query", .type = "KQuery", .required = true});
    meta.params.push_back(
      ParamMeta{.name = "ind", .type = "Indicator", .required = true, .is_part_param = true});
    meta.params.push_back(ParamMeta{.name = "mode", .type = "int", .required = true});
    meta.params.push_back(ParamMeta{.name = "fill_null", .type = "bool", .default_value = "true"});
    registerIndicatorPart(
      "core.ind.insum", meta, [](const Parameter& sc, const PartParamMap& pp) -> Part {
          return INSUM(sc.get<Block>("block"), sc.get<KQuery>("query"),
                       std::get<Indicator>(pp.at("ind").at(0)),
                       static_cast<int>(sc.get<int64_t>("mode")), sc.get<bool>("fill_null"));
      });
}

}  // namespace hku