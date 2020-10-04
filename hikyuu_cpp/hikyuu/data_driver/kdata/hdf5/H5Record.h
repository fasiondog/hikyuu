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
    uint64_t datetime;
    uint32_t openPrice;
    uint32_t highPrice;
    uint32_t lowPrice;
    uint32_t closePrice;
    uint64_t transAmount;
    uint64_t transCount;
};

struct H5IndexRecord {
    uint64_t datetime;
    uint64_t start;
};

struct H5TimeLineRecord {
    uint64_t datetime;
    uint64_t price;
    uint64_t vol;
};

struct H5TransRecord {
    uint64_t datetime;
    uint64_t price;
    uint64_t vol;
    uint8_t buyorsell;
};

}  // namespace hku

#endif /* H5RECORD_H_ */
