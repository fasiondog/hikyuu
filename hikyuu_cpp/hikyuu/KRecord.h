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
 * K线数据记录
 * @ingroup StockManage
 */
class KRecord {
public:
    Datetime datetime;    ///< 日期，格式：YYYYMMDDHHMM 如：200901010930
    price_t openPrice;    ///< 开盘价
    price_t highPrice;    ///< 最高价
    price_t lowPrice;     ///< 最低价
    price_t closePrice;   ///< 收盘价
    price_t transAmount;  ///< 成交金额（千元）
    price_t transCount;   ///< 成交量（手），日线以下为股数

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

    bool isValid() const {
        return datetime == Null<Datetime>() ? false : true;
    }
};

/** @ingroup StockManage */
typedef vector<KRecord> KRecordList;

/** @ingroup StockManage */
typedef shared_ptr<KRecordList> KRecordListPtr;

/**
 * 输出KRecord信息，如：KRecord(datetime, open, high, low, close, transAmount, count)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const KRecord&);

/**
 * 比较两个KRecord是否相等，一般仅测试时使用
 * @ingroup StockManage
 */
bool HKU_API operator==(const KRecord& d1, const KRecord& d2);

/**
 * 两个KRecord不等比较
 * @ingroup StockManage
 */
bool HKU_API operator!=(const KRecord& d1, const KRecord& d2);

}  // namespace hku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::KRecord> : ostream_formatter {};
#endif

#endif /* KRECORD_H_ */
