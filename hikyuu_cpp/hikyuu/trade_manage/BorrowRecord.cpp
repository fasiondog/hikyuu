/*
 * BorrowRecord.cpp
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#include "BorrowRecord.h"

namespace hku {

HKU_API std::ostream& operator<<(std::ostream& os, const BorrowRecord& bor) {
    string strip(", ");
    os << "BorrowRecord(" << bor.stock << strip << bor.number << strip << bor.value << strip << ")";
    return os;
}

BorrowRecord::BorrowRecord() : number(0.0), value(0.0) {}

BorrowRecord ::BorrowRecord(const Stock& stock, double number, price_t value)
: stock(stock), number(number), value(value) {}

BorrowRecord::BorrowRecord(const BorrowRecord& other)
: stock(other.stock), number(other.number), value(other.value), record_list(other.record_list) {}

BorrowRecord::BorrowRecord(BorrowRecord&& other)
: stock(std::move(other.stock)),
  number(other.number),
  value(other.value),
  record_list(std::move(other.record_list)) {
    other.number = 0.0;
    other.value = 0.0;
}

BorrowRecord& BorrowRecord::operator=(const BorrowRecord& other) {
    HKU_IF_RETURN(this == &other, *this);
    stock = other.stock;
    number = other.number;
    value = other.value;
    return *this;
}

BorrowRecord& BorrowRecord::operator=(BorrowRecord&& other) {
    HKU_IF_RETURN(this == &other, *this);
    stock = std::move(other.stock);
    number = other.number;
    value = other.value;
    record_list = std::move(other.record_list);
    other.number = 0.0;
    other.value = 0.0;
    return *this;
}

} /* namespace hku */
