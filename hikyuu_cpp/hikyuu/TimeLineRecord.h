/*
 * TimeLineRecord.h
 *
 *  Created on: 2019-1-27
 *      Author: fasiondog
 */

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

    virtual ~TimeLineRecord();

    bool isValid() const;
};

/** @ingroup StockManage */
typedef vector<TimeLineRecord> TimeLine;


/** @ingroup StockManage */
typedef shared_ptr<TimeLineRecord> TimeLinePtr;


/**
 * 输出TimeSharingRecord信息，如：TimeSharingRecord(datetime, price, vol)
 * @ingroup StockManage
 */
HKU_API std::ostream & operator<<(std::ostream &, const TimeLineRecord&);


/**
 * 比较两个TimeSharingRecord是否相等，一般仅测试时使用
 * @ingroup StockManage
 */
bool HKU_API operator==(const TimeLineRecord& d1, const TimeLineRecord&d2);


} /* namespace hku */

#endif /* TIMELINERECORD_H_ */
