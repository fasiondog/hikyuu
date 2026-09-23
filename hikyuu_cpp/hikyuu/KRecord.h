/*
 * KRecord.h
 *
 *  Created on: 2011-12-24
 *      Author: fasiondog
 */

#pragma once
#ifndef KRECORD_H_
#define KRECORD_H_

#include "DataType.h"

namespace hku {

/**
 * K-line data record
 * @ingroup StockManage
 */
class HKU_API KRecord {
public:
    Datetime datetime;    ///< Date, format: YYYYMMDDHHMM, e.g. 200901010930
    price_t openPrice;    ///< Open price
    price_t highPrice;    ///< High price
    price_t lowPrice;     ///< Low price
    price_t closePrice;   ///< Close price
    price_t transAmount;  ///< Turnover amount (in units of 10 thousand yuan)
    price_t transCount;   ///< Trading volume (in lots); below the daily line it is the share count

    KRecord()
    : datetime(Null<Datetime>()),
      openPrice(0.0),
      highPrice(0.0),
      lowPrice(0.0),
      closePrice(0.0),
      transAmount(0.0),
      transCount(0.0) {}

    explicit KRecord(const Datetime& indate)
    : datetime(indate),
      openPrice(0.0),
      highPrice(0.0),
      lowPrice(0.0),
      closePrice(0.0),
      transAmount(0.0),
      transCount(0.0) {}

    KRecord(const Datetime& date, price_t openPrice, price_t highPrice, price_t lowPrice,
            price_t closePrice, price_t transAmount, price_t transCount)
    : datetime(date),
      openPrice(openPrice),
      highPrice(highPrice),
      lowPrice(lowPrice),
      closePrice(closePrice),
      transAmount(transAmount),
      transCount(transCount) {}

    bool isValid() const noexcept {
        return datetime == Null<Datetime>() ? false : true;
    }

public:
    static const KRecord NullKRecord;
};

/** @ingroup StockManage */
typedef vector<KRecord> KRecordList;

/** @ingroup StockManage */
typedef shared_ptr<KRecordList> KRecordListPtr;

/**
 * Output the KRecord information, e.g. KRecord(datetime, open, high, low, close, transAmount,
 * count)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const KRecord&);

/**
 * Compare whether two KRecord are equal, generally used in tests only
 * @ingroup StockManage
 */
bool HKU_API operator==(const KRecord& d1, const KRecord& d2);

/**
 * Unequal comparison of two KRecord
 * @ingroup StockManage
 */
bool HKU_API operator!=(const KRecord& d1, const KRecord& d2);

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::KRecord> : ostream_formatter {};
#endif

#endif /* KRECORD_H_ */
