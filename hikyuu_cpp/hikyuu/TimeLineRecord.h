/*
 * TimeLineRecord.h
 *
 *  Created on: 2019-1-27
 *      Author: fasiondog
 */

#pragma once
#ifndef TIMELINERECORD_H_
#define TIMELINERECORD_H_

#include "DataType.h"

namespace hku {

/**
 * 分时线记录
 * @ingroup StockManage
 */
class HKU_API TimeLineRecord {
public:
    Datetime datetime;
    price_t price;
    price_t vol;

    TimeLineRecord();
    TimeLineRecord(const Datetime& datetime, price_t price, price_t vol);

    bool isValid() const;
};

/**
 * 分时线
 * @ingroup StockManage
 */
typedef vector<TimeLineRecord> TimeLineList;

/**
 * 输出TimeLineRecord信息，如：TimeSharingRecord(datetime, price, vol)
 * @ingroup StockManage
 */
HKU_API std::ostream& operator<<(std::ostream&, const TimeLineRecord&);

/**
 * 输出TimeLine信息
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
HKU_API std::ostream& operator<<(std::ostream& os, const TimeLineList&);

/**
 * 比较两个TimeLineRecord是否相等，一般仅测试时使用
 * @ingroup StockManage
 */
bool HKU_API operator==(const TimeLineRecord& d1, const TimeLineRecord& d2);

} /* namespace hku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hku::TimeLineRecord> : ostream_formatter {};
#endif

#endif /* TIMELINERECORD_H_ */
