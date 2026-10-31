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
 * Cross-sectional statistics (it returns the number of the stocks in the block)
 * @param blk the block to be counted
 * @param query the statistics range
 * @return Indicator
 */
Indicator HKU_API BLOCKSETNUM(const Block& blk, const KQuery& query);
Indicator HKU_API BLOCKSETNUM(const Block& blk);

inline void registerPart_BLOCKSETNUM() {
    PartMeta meta;
    meta.type = PartType::IND;
    meta.name = "BLOCKSETNUM";
    meta.params.push_back(ParamMeta{.name = "blk", .type = "Block", .required = true});
    meta.params.push_back(ParamMeta{.name = "query", .type = "KQuery", .required = true});
    registerIndicatorPart("core.ind.blocksetnum", meta,
                          [](const Parameter& sc, const PartParamMap&) -> Part {
                              return BLOCKSETNUM(sc.get<Block>("blk"), sc.get<KQuery>("query"));
                          });
}

}  // namespace hku