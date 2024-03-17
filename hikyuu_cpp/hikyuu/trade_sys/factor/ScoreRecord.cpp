/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: fasiondog
 */

#include "ScoreRecord.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& out, const ScoreRecord& td) {
    out << std::fixed;
    out.precision(4);
    out << "(" << td.stock.market_code() << ", " << td.value << ")" << std::endl;
    out.unsetf(std::ostream::floatfield);
    out.precision();
    return out;
}

HKU_API std::ostream& operator<<(std::ostream& out, const ScoreRecordList& td) {
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
        out << "......" << std::endl;
        for (size_t i = total - 5; i < total; i++) {
            out << i << ": " << td[i];
        }
    }
    out.unsetf(std::ostream::floatfield);
    out.precision();
    return out;
}

HKU_API std::ostream& operator<<(std::ostream& out, const vector<ScoreRecordList>& td) {
    out << std::fixed;
    out.precision(4);
    size_t total = td.size();
    if (total <= 2) {
        for (size_t i = 0; i < total; i++) {
            out << "========= " << i << " =========" << std::endl;
            out << td[i];
        }
    } else {
        out << "========= 0 =========" << std::endl;
        out << td[0];
        out << "......" << std::endl;
        out << "......" << std::endl;
        out << "========= " << total - 1 << " =========" << std::endl;
        out << td[total - 1];
    }

    out.unsetf(std::ostream::floatfield);
    out.precision();
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