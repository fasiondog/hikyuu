/*
 * TransRecord.h
 *
 *  Created on: 2019年2月10日
 *      Author: fasiondog
 */

#pragma once
#ifndef TRANSRECORD_H_
#define TRANSRECORD_H_

#include "DataType.h"

namespace hku {

/**
 * 分笔成交记录
 * @ingroup StockManage
 */
class HKU_API TransRecord {
public:
    enum DIRECT {
        BUY = 0,    /**< 买盘 */
        SELL = 1,   /**< 卖盘 */
        AUCTION = 2 /**< 集合竞价 */
    };

    Datetime datetime;  // 时间
    price_t price;      // 成交均价
    price_t vol;        // 成交量
    DIRECT direct;      // 买卖盘性质：1--sell 0--buy 2--集合竞价

    TransRecord();
    TransRecord(const Datetime& datetime, price_t price, price_t vol, DIRECT);

    bool isValid() const;
};

/**
 * 分时线
 * @ingroup StockManage
 */
typedef vector<TransRecord> TransList;
typedef vector<TransRecord> TransRecordList;

/**
 * 输出 TransRecord 信息，如：TimeSharingRecord(datetime, price, vol)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const TransRecord&);

/**
 * 输出 TransList 信息
 * @details
 * <pre>
 * TimeLine{
 *   size : 738501
 *   start: YYYY-MM-DD hh:mm:ss
 *   last : YYYY-MM-DD hh:mm:ss
 *  }
 * </pre>
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream& os, const TransList&);

/**
 * 比较两个 TransRecord 是否相等，一般仅测试时使用
 * @ingroup StockManage
 */
bool HKU_API operator==(const TransRecord& d1, const TransRecord& d2);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TransRecord> : ostream_formatter {};
#endif

#endif /* TRANSRECORD_H_ */
