/*
 * TimeSharingRecord.h
 *
 *  Created on: 2019年1月27日
 *      Author: fasiondog
 */

#ifndef TIMESHARINGRECORD_H_
#define TIMESHARINGRECORD_H_

#include "DataType.h"

namespace hku {

/**
 * 分时线记录
 * @ingroup StockManage
 */
class HKU_API TimeSharingRecord {
public:
    Datetime datetime;
    price_t price;
    price_t vol;

    TimeSharingRecord();
    TimeSharingRecord(Datetime datetime, price_t price, price_t vol);

    virtual ~TimeSharingRecord();

    bool isValid() const;
};

/** @ingroup StockManage */
typedef vector<TimeSharingRecord> TimeSharingList;


/** @ingroup StockManage */
typedef shared_ptr<TimeSharingRecord> TimeSharingListPtr;


/**
 * 输出TimeSharingRecord信息，如：TimeSharingRecord(datetime, price, vol)
 * @ingroup StockManage
 */
HKU_API std::ostream & operator<<(std::ostream &, const TimeSharingRecord&);


/**
 * 比较两个TimeSharingRecord是否相等，一般仅测试时使用
 * @ingroup StockManage
 */
bool HKU_API operator==(const TimeSharingRecord& d1, const TimeSharingRecord&d2);


} /* namespace hku */

#endif /* TIMESHARINGRECORD_H_ */
