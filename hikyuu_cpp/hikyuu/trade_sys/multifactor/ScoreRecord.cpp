/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: fasiondog
 */

#include "ScoreRecord.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& out, const ScoreRecord& td) {
    auto oldFlags = out.flags();
    auto oldPrecision = out.precision();
    out << std::fixed;
    out.precision(4);
    out << "(" << td.stock.market_code() << ", " << td.value << ")" << '\n';
    out.flags(oldFlags);
    out.precision(oldPrecision);
    return out;
}

HKU_API std::ostream& operator<<(std::ostream& out, const ScoreRecordList& td) {
    auto oldFlags = out.flags();
    auto oldPrecision = out.precision();
    out << std::fixed;
    out.precision(4);
    size_t total = td.size();
    if (total < 10) {
        for (size_t i = 0; i < total; i++) {
            out << i << ": " << td[i];
        }
    } else {
        for (size_t i = 0; i < 5; i++) {
            out << i << ": " << td[i];
        }
        out << "......" << '\n';
        for (size_t i = total - 5; i < total; i++) {
            out << i << ": " << td[i];
        }
    }
    out.flags(oldFlags);
    out.precision(oldPrecision);
    return out;
}

HKU_API std::ostream& operator<<(std::ostream& out, const vector<ScoreRecordList>& td) {
    auto oldFlags = out.flags();
    auto oldPrecision = out.precision();
    out << std::fixed;
    out.precision(4);
    size_t total = td.size();
    if (total <= 2) {
        for (size_t i = 0; i < total; i++) {
            out << "========= " << i << " =========" << '\n';
            out << td[i];
        }
    } else {
        out << "========= 0 =========" << '\n';
        out << td[0];
        out << "......" << '\n';
        out << "......" << '\n';
        out << "========= " << total - 1 << " =========" << '\n';
        out << td[total - 1];
    }
    out.flags(oldFlags);
    out.precision(oldPrecision);
    return out;
}

ScoreRecord::ScoreRecord(const Stock& stock_, value_t value_) : stock(stock_), value(value_) {}

ScoreRecord::ScoreRecord(const ScoreRecord& other) : stock(other.stock), value(other.value) {}

ScoreRecord::ScoreRecord(ScoreRecord&& rv) : stock(std::move(rv.stock)), value(rv.value) {}

ScoreRecord& ScoreRecord::operator=(const ScoreRecord& rv) {
    HKU_IF_RETURN(this == &rv, *this);
    stock = rv.stock;
    value = rv.value;
    return *this;
}

ScoreRecord& ScoreRecord::operator=(ScoreRecord&& rv) {
    HKU_IF_RETURN(this == &rv, *this);
    stock = std::move(rv.stock);
    value = rv.value;
    return *this;
}

}  // namespace hku