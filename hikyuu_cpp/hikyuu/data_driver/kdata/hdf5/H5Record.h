/*
 * H5Record.h
 *
 *  Created on: 2012-9-16
 *      Author: fasiondog
 */

#ifndef H5RECORD_H_
#define H5RECORD_H_

#include <H5Cpp.h>

#include "../../../DataType.h"

namespace hku {

typedef shared_ptr<H5::H5File> H5FilePtr;

struct H5Record {
    hku_uint64 datetime;
    hku_uint32 openPrice;
    hku_uint32 highPrice;
    hku_uint32 lowPrice;
    hku_uint32 closePrice;
    hku_uint64 transAmount;
    hku_uint64 transCount;
};

struct H5IndexRecord {
    hku_uint64 datetime;
    hku_uint64 start;
};

struct H5TimeLineRecord {
    hku_uint64 datetime;
    hku_uint64 price;
    hku_uint64 vol;
};

struct H5TransRecord {
    hku_uint64 datetime;
    hku_uint64 price;
    hku_uint64 vol;
    hku_uint8  buyorsell;
};

} /* namespae */

#endif /* H5RECORD_H_ */
