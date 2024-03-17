/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/Stock.h"
#include "hikyuu/indicator/Indicator.h"

namespace hku {

struct HKU_API ScoreRecord {
    typedef Indicator::value_t value_t;

    Stock stock;
    value_t value;

    ScoreRecord() = default;
    ScoreRecord(const Stock& stock_, value_t value_);

    ScoreRecord(const ScoreRecord&);
    ScoreRecord(ScoreRecord&&);

    ScoreRecord& operator=(const ScoreRecord&);
    ScoreRecord& operator=(ScoreRecord&&);
};

typedef vector<ScoreRecord> ScoreRecordList;

HKU_API std::ostream& operator<<(std::ostream& out, const ScoreRecord& td);

HKU_API std::ostream& operator<<(std::ostream& out, const ScoreRecordList& td);

HKU_API std::ostream& operator<<(std::ostream& out, const vector<ScoreRecordList>& td);

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::ScoreRecord> : ostream_formatter {};

template <>
struct fmt::formatter<hku::ScoreRecordList> : ostream_formatter {};

template <>
struct fmt::formatter<std::vector<hku::ScoreRecordList>> : ostream_formatter {};
#endif