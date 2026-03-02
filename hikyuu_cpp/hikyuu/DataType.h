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
#include "utilities/config.h"
#include "lang.h"

#if HKU_OS_OSX
#include <MacTypes.h>
#endif

#if HKU_SUPPORT_SERIALIZATION
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/list.hpp>

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

// MSVC 的 __cpluscplus 在 VS2017 之后才有正确的版本号，且只支持 C++17 及以上, 详见：
// https://docs.microsoft.com/zh-cn/cpp/build/reference/zc-cplusplus?view=vs-2019
#if CPP_STANDARD < CPP_STANDARD_11
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
    return std::abs(num) < std::numeric_limits<price_t>::epsilon();
}

using fmt::format;

/** @} */

}  // namespace hku

#if HKU_SUPPORT_SERIALIZATION
// 跨平台序列化支持
//-------------------
// std::map 特化
//-------------------
namespace boost {
namespace serialization {
template <class Archive, class Key, class Value, class Compare, class Alloc>
void serialize(Archive& ar, std::map<Key, Value, Compare, Alloc>& m, const unsigned int version);
}
}  // namespace boost

namespace boost {
namespace serialization {

template <class Archive, class Key, class Value, class Compare, class Alloc>
void save(Archive& ar, const std::map<Key, Value, Compare, Alloc>& m, const unsigned int version) {
    uint32_t count = static_cast<uint32_t>(m.size());
    ar& make_nvp("count", count);
    for (const auto& kv : m) {
        ar& make_nvp("key", kv.first);
        ar& make_nvp("value", kv.second);
    }
}

template <class Archive, class Key, class Value, class Compare, class Alloc>
void load(Archive& ar, std::map<Key, Value, Compare, Alloc>& m, const unsigned int version) {
    m.clear();

    uint32_t count;
    ar& boost::make_nvp("count", count);

    for (uint32_t i = 0; i < count; ++i) {
        Key k;
        Value v;
        ar& make_nvp("key", k);
        ar& make_nvp("value", v);
        m.insert(std::make_pair(k, v));
    }
}

template <class Archive, class Key, class Value, class Compare, class Alloc>
void serialize(Archive& ar, std::map<Key, Value, Compare, Alloc>& m, const unsigned int version) {
    split_free(ar, m, version);
}

}  // namespace serialization
}  // namespace boost

//-----------------------
// std::unordered_map 特化
//-----------------------

namespace boost {
namespace serialization {
template <class Archive, class Key, class Value, class Compare, class Alloc>
void serialize(Archive& ar, std::unordered_map<Key, Value, Compare, Alloc>& m,
               const unsigned int version);
}
}  // namespace boost

namespace boost {
namespace serialization {

template <class Archive, class Key, class Value, class Compare, class Alloc>
void save(Archive& ar, const std::unordered_map<Key, Value, Compare, Alloc>& m,
          const unsigned int version) {
    uint32_t count = static_cast<uint32_t>(m.size());
    ar& make_nvp("count", count);
    for (const auto& kv : m) {
        ar& make_nvp("key", kv.first);
        ar& make_nvp("value", kv.second);
    }
}

template <class Archive, class Key, class Value, class Compare, class Alloc>
void load(Archive& ar, std::unordered_map<Key, Value, Compare, Alloc>& m,
          const unsigned int version) {
    m.clear();

    uint32_t count;
    ar& boost::make_nvp("count", count);

    for (uint32_t i = 0; i < count; ++i) {
        Key k;
        Value v;
        ar& make_nvp("key", k);
        ar& make_nvp("value", v);
        m.insert(std::make_pair(k, v));
    }
}

template <class Archive, class Key, class Value, class Compare, class Alloc>
void serialize(Archive& ar, std::unordered_map<Key, Value, Compare, Alloc>& m,
               const unsigned int version) {
    split_free(ar, m, version);
}

}  // namespace serialization
}  // namespace boost

//-----------------------
// std::set unordered_set 特化
//-----------------------

namespace boost {
namespace serialization {
template <class Archive, class Key, class Compare, class Alloc>
void save(Archive& ar, const std::set<Key, Compare, Alloc>& s, const unsigned int version) {
    uint32_t count = static_cast<uint32_t>(s.size());
    ar& ::boost::make_nvp("count", count);
    for (const auto& item : s) {
        ar& ::boost::make_nvp("item", item);
    }
}

template <class Archive, class Key, class Compare, class Alloc>
void load(Archive& ar, std::set<Key, Compare, Alloc>& s, const unsigned int version) {
    s.clear();
    uint32_t count;
    ar& ::boost::make_nvp("count", count);
    for (uint32_t i = 0; i < count; ++i) {
        Key item;  // 自动推导 Key 类型
        ar& ::boost::make_nvp("item", item);
        s.insert(item);
    }
}

template <class Archive, class Key, class Compare, class Alloc>
void serialize(Archive& ar, std::set<Key, Compare, Alloc>& s, const unsigned int version) {
    split_free(ar, s, version);
}

template <class Archive, class Key, class Hash, class KeyEqual, class Alloc>
void save(Archive& ar, const std::unordered_set<Key, Hash, KeyEqual, Alloc>& s,
          const unsigned int version) {
    uint32_t count = static_cast<uint32_t>(s.size());
    ar& ::boost::make_nvp("count", count);
    for (const auto& item : s) {
        ar& ::boost::make_nvp("item", item);
    }
}

template <class Archive, class Key, class Hash, class KeyEqual, class Alloc>
void load(Archive& ar, std::unordered_set<Key, Hash, KeyEqual, Alloc>& s,
          const unsigned int version) {
    s.clear();
    uint32_t count;
    ar& ::boost::make_nvp("count", count);
    for (uint32_t i = 0; i < count; ++i) {
        Key item;
        ar& ::boost::make_nvp("item", item);
        s.insert(item);
    }
}

template <class Archive, class Key, class Hash, class KeyEqual, class Alloc>
void serialize(Archive& ar, std::unordered_set<Key, Hash, KeyEqual, Alloc>& s,
               const unsigned int version) {
    split_free(ar, s, version);
}

}  // namespace serialization
}  // namespace boost

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* DATATYPE_H_ */
