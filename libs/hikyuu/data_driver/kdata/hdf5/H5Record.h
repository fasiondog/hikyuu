/*
 * H5Record.h
 *
 *  Created on: 2012-9-16
 *      Author: fasiondog
 */

#ifndef H5RECORD_H_
#define H5RECORD_H_

#include "../../../DataType.h"

#if defined(BOOST_MSVC)
#include <cpp/H5Cpp.h>
#else
#include <hdf5/serial/H5Cpp.h>
#endif

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

struct H5IndexRecord{
    hku_uint64 datetime;
    hku_uint64 start;
};

} /* namespae */

#endif /* H5RECORD_H_ */
