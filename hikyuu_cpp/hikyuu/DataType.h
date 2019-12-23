/*
 * DataType.h
 *
 * Created on: 2009-11-8
 *      Author: fasiondog
 */

#pragma once
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
#include "datetime/Datetime.h"
#include "utilities/Null.h"
#include "utilities/arithmetic.h"
#include "utilities/SpendTimer.h"
#include "utilities/exception.h"

#if defined(_MSC_VER)
#pragma warning(disable : 4251)
#pragma warning(disable : 4290)
#pragma warning(disable : 4996)
#endif

namespace hku {

/**
 * @ingroup DataType
 * @{
 */

#if !defined(int64)
typedef long long int64;
#endif

#if !defined(uint64)
typedef unsigned long long uint64;
#endif

#if !defined(int32)
typedef int int32;
#endif

#if !defined(uint32)
typedef unsigned uint32;
#endif

#if !defined(int8)
typedef char int8;
#endif

#if !defined(uint8)
typedef unsigned char uint8;
#endif

typedef double price_t;

using std::string;

using std::enable_shared_from_this;
using std::make_shared;
using std::shared_ptr;

using std::list;
using std::map;
using std::unordered_map;
using std::vector;

typedef vector<price_t> PriceList;
typedef vector<string> StringList;

using std::isfinite;
using std::isinf;
using std::isnan;

using fmt::format;

/** @} */

}  // namespace hku

#endif /* DATATYPE_H_ */
