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

#if 0
// 进入 boost::serialization 命名空间进行“埋雷”
namespace boost {
namespace serialization {

// ============================================================
// 1. 针对 'long' 的毒药特化
// ============================================================
template <class Archive>
void serialize(Archive& ar, long& t, const unsigned int version) {
    // 只要编译器尝试实例化这个函数（即尝试序列化 long），就会触发此断言
    static_assert(
      sizeof(long) == -1,
      "🚨 FATAL ERROR: 禁止直接序列化 'long' 类型！\n"
      "原因：'long' 在 Linux (8字节) 和 Windows (4字节) 上大小不同，会导致跨平台崩溃。\n"
      "解决：请将成员变量类型改为 'int32_t' 或 'int64_t'。\n"
      "位置：请查看调用栈上方报错的变量。");
}

// 针对 const long (防止 const 成员变量绕过)
template <class Archive>
void serialize(Archive& ar, const long& t, const unsigned int version) {
    static_assert(sizeof(long) == -1,
                  "🚨 FATAL ERROR: 禁止序列化 'const long'！请改为 int32_t/int64_t。");
}

// ============================================================
// 2. 针对 'unsigned long' 的毒药特化
// ============================================================
template <class Archive>
void serialize(Archive& ar, unsigned long& t, const unsigned int version) {
    static_assert(sizeof(unsigned long) == -1,
                  "🚨 FATAL ERROR: 禁止直接序列化 'unsigned long' 类型！\n"
                  "原因：跨平台大小不一致 (Linux 8字节 vs Windows 4字节)。\n"
                  "解决：请改为 'uint32_t' 或 'uint64_t'。");
}

// ============================================================
// 3. 针对 'size_t' 的警告 (可选，视严格程度而定)
// ============================================================
// 如果你确定只在 64 位系统间传输，可以注释掉下面这段。
// 如果要绝对安全（兼容 32 位），则开启。
/*
template<class Archive>
void serialize(Archive& ar, std::size_t& t, const unsigned int version) {
    static_assert(sizeof(std::size_t) == -1,
        "⚠️ WARNING: 检测到序列化 'size_t'。\n"
        "虽然 64 位系统间通常安全，但为了绝对跨平台兼容，建议改为 uint32_t 或 uint64_t。"
    );
}
*/

// ============================================================
// 4. 针对 'time_t' 的毒药特化 (常见隐患)
// ============================================================
// #include <ctime>
// template <class Archive>
// void serialize(Archive& ar, std::time_t& t, const unsigned int version) {
//     static_assert(sizeof(std::time_t) == -1,
//                   "🚨 FATAL ERROR: 禁止直接序列化 'time_t'！\n"
//                   "原因：'time_t' 在不同平台/编译器下实现不同 (可能是 long, long long, 或
//                   int)。\n" "解决：请转换为 'int64_t' (秒时间戳) 后再序列化。");
// }

}  // namespace serialization
}  // namespace boost
#endif

#endif /* HKU_SUPPORT_SERIALIZATION */

#endif /* DATATYPE_H_ */
