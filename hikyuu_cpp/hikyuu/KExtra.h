/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-03
 *      Author: fasiondog
 */

#pragma once

#include "DataType.h"
#include "KRecord.h"

namespace hku {

struct KExtra {
    string ktype;
    string basetype;
    std::function<Datetime(const Datetime&)> getPhaseEnd;
    std::function<int32_t()> getMinutes;

    KExtra() = default;
    KExtra(const KExtra&) = default;
    KExtra& operator=(const KExtra&) = default;

    KExtra(KExtra&&);
    KExtra& operator=(KExtra&&);

    KRecordList getExtraKRecordList(const KRecordList& kdata) const;
};

void HKU_API registerKTypeExtra(const string& ktype, const string& basetype,
                                const std::function<Datetime(const Datetime&)>& getPhaseEnd,
                                const std::function<int32_t()>& getMinutes);

bool HKU_API isExtraKType(const string& ktype);

void HKU_API releaseKExtra();

const KExtra& getKExtra(const string& ktype);

}  // namespace hku