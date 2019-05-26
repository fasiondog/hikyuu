/*
 * DataType.h
 *
 * Created on: 2009-11-8
 *      Author: fasiondog
 */

#ifndef DATATYPE_H_
#define DATATYPE_H_

#ifndef HKU_API
#define HKU_API
#endif

#include <boost/config.hpp>
//#include <boost/shared_ptr.hpp>
//#include <boost/make_shared.hpp>
//#include <boost/enable_shared_from_this.hpp>

#include <stdio.h>
#include <iostream>
#include <list>
#include <cmath>
#include <memory>
//#include <vector>
#include <map>
#include <unordered_map>

#include "Log.h"
#include "utilities/Null.h"
#include "datetime/Datetime.h"

#if defined(BOOST_MSVC)
#pragma warning (disable: 4251)
#pragma warning (disable: 4290)
#pragma warning (disable: 4996)
#endif

namespace hku {

/**
 * @ingroup DataType
 * @{
 */

typedef long long hku_int64;
typedef unsigned long long hku_uint64;
typedef int hku_int32;
typedef unsigned hku_uint32;
typedef char hku_int8;
typedef unsigned char hku_uint8;


typedef double price_t;

typedef std::string string;

using std::shared_ptr;
using std::make_shared;
using std::enable_shared_from_this;

using std::vector;
using std::list;
using std::map;
using std::unordered_map;

typedef vector<price_t> PriceList;
typedef vector<string> StringList;

using std::isnan;
using std::isinf;
using std::isfinite;

/** @} */

} /*namespace*/

#endif /* DATATYPE_H_ */
