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

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "config.h"
#include <boost/config.hpp>

#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <list>
#include <cmath>
#include <memory>
// #include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include "utilities/Log.h"
#include "utilities/osdef.h"
#include "utilities/cppdef.h"
#include "utilities/datetime/Datetime.h"
#include "utilities/Null.h"
#include "utilities/arithmetic.h"
#include "utilities/SpendTimer.h"

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/unordered_set.hpp>

#if HKU_SUPPORT_XML_ARCHIVE
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#endif /* HKU_SUPPORT_XML_ARCHIVE */

#if HKU_SUPPORT_TEXT_ARCHIVE
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif /* HKU_SUPPORT_TEXT_ARCHIVE */

#if HKU_SUPPORT_BINARY_ARCHIVE
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#endif /* HKU_SUPPORT_BINARY_ARCHIVE */

#include <boost/serialization/export.hpp>
#endif /* HKU_SUPPORT_SERIALIZATION */

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

// MSVC 的 __cpluscplus 之在 VS2017 之后才有正确的版本号，且只支持 C++17 及以上, 详见：
// https://docs.microsoft.com/zh-cn/cpp/build/reference/zc-cplusplus?view=vs-2019
#if defined(_MSC_VER) || __cplusplus < 201103L
#if !defined(int64_t)
typedef long long int64_t;
#endif

#if !defined(uint64_t)
typedef unsigned long long uint64_t;
#endif

#if !defined(int32_t)
typedef int int32_t;
#endif

#if !defined(uint32_t)
typedef unsigned uint32_t;
#endif

#if !defined(int8_t)
typedef char int8_t;
#endif

#if !defined(uint8_t)
typedef unsigned char uint8_t;
#endif

#else
using std::int32_t;
using std::int64_t;
using std::int8_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;

#endif /* #if defined(_MSC_VER) || __cplusplus < 201103L */

#ifndef MAX_DOUBLE
#define MAX_DOUBLE std::numeric_limits<double>::max()
#endif

typedef double price_t;

using std::string;

using std::enable_shared_from_this;
using std::make_shared;
using std::make_unique;
using std::shared_ptr;
using std::unique_ptr;

using std::list;
using std::map;
using std::unordered_map;
using std::vector;

typedef vector<price_t> PriceList;
typedef vector<string> StringList;

using std::isfinite;
using std::isinf;
using std::isnan;

inline bool iszero(price_t num) {
    const price_t epsilon = std::numeric_limits<price_t>::epsilon();
    return std::abs(num) < epsilon;
}

using fmt::format;

inline std::ostream &operator<<(std::ostream &os, const PriceList &p) {
    size_t len = p.size();
    const size_t print = 3;
    os << "[";
    for (size_t i = 0; i < len; i++) {
        if ((i < print) || (i >= len - print)) {
            os << p[i];
            if (i != len - 1)
                os << ", ";
        } else if (i == 3)
            os << "..., ";
    }
    os << "]";
    return os;
}

/** @} */

}  // namespace hku

#endif /* DATATYPE_H_ */
