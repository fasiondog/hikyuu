/*
 * H5Record.h
 *
 *  Created on: 2012-9-16
 *      Author: fasiondog
 */

#pragma once
#ifndef H5RECORD_H_
#define H5RECORD_H_

#include <H5Cpp.h>

#include "../../../DataType.h"

namespace hku {

typedef shared_ptr<H5::H5File> H5FilePtr;

struct H5Record {
    uint64 datetime;
    uint32 openPrice;
    uint32 highPrice;
    uint32 lowPrice;
    uint32 closePrice;
    uint64 transAmount;
    uint64 transCount;
};

struct H5IndexRecord {
    uint64 datetime;
    uint64 start;
};

struct H5TimeLineRecord {
    uint64 datetime;
    uint64 price;
    uint64 vol;
};

struct H5TransRecord {
    uint64 datetime;
    uint64 price;
    uint64 vol;
    uint8  buyorsell;
};

} /* namespae */

#endif /* H5RECORD_H_ */
