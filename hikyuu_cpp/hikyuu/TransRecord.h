/*
 * TransRecord.h
 *
 *  Created on: 2019年2月10日
 *      Author: fasiondog
 */

#ifndef TRANSRECORD_H_
#define TRANSRECORD_H_

#include "DataType.h"

namespace hku {

class TransRecord {
public:
    Datetime datetime;  //时间
    price_t price;      //成交均价
    price_t vol;        //成交量
    hku_int64 buyorsell;  //买卖盘性质：1--sell 0--buy 2--集合竞价

    TransRecord();
    virtual ~TransRecord();
};

} /* namespace hku */

#endif /* TRANSRECORD_H_ */
