/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-01-15
 *      Author: fasiondog
 */

#pragma once

#include <string>

#include "osdef.h"
#include "cppdef.h"
#if !HKU_OS_IOS && CPP_STANDARD >= CPP_STANDARD_17
#include <any>
#else
#include <boost/any.hpp>
#endif

#if defined(HKU_SUPPORT_DATETIME)
#include "hikyuu/utilities/datetime/Datetime.h"
#endif

namespace hku {

#if !HKU_OS_IOS && CPP_STANDARD >= CPP_STANDARD_17
using any_t = std::any;
using std::any_cast;
#else
using any_t = boost::any;
using boost::any_cast;
#endif

//------------------------------------------------------------------------------
//
// 常见基本类型包装的 any_t 和 std::string 的互相转换函数
// any_to_string 要用户自定义类型需包含从 std::string 进行构造的构造函数
// string_to_any 需要用户自定义实现 std::to_string 特化方法
//
//------------------------------------------------------------------------------

template <typename ValueT>
inline std::string any_to_string(const any_t& data) {
    return any_cast<ValueT>(data).str();
}

template <typename ValueT>
inline any_t string_to_any(const std::string& data) {
    return any_t(ValueT(data));
}

template <>
inline std::string any_to_string<int>(const any_t& data) {
    return std::to_string(any_cast<int>(data));
}

template <>
inline std::string any_to_string<long>(const any_t& data) {
    return std::to_string(any_cast<long>(data));
}

template <>
inline std::string any_to_string<long long>(const any_t& data) {
    return std::to_string(any_cast<long long>(data));
}

template <>
inline std::string any_to_string<unsigned int>(const any_t& data) {
    return std::to_string(any_cast<unsigned int>(data));
}

template <>
inline std::string any_to_string<unsigned long>(const any_t& data) {
    return std::to_string(any_cast<unsigned long>(data));
}

template <>
inline std::string any_to_string<unsigned long long>(const any_t& data) {
    return std::to_string(any_cast<unsigned long long>(data));
}

template <>
inline std::string any_to_string<float>(const any_t& data) {
    return std::to_string(any_cast<float>(data));
}

template <>
inline std::string any_to_string<double>(const any_t& data) {
    return std::to_string(any_cast<double>(data));
}

template <>
inline std::string any_to_string<long double>(const any_t& data) {
    return std::to_string(any_cast<long double>(data));
}

template <>
inline any_t string_to_any<int>(const std::string& data) {
    return any_t(std::stoi(data));
}

template <>
inline any_t string_to_any<long>(const std::string& data) {
    return any_t(std::stol(data));
}

template <>
inline any_t string_to_any<long long>(const std::string& data) {
    return any_t(std::stoll(data));
}

template <>
inline any_t string_to_any<unsigned int>(const std::string& data) {
    return any_t((unsigned int)(std::stoul(data)));
}

template <>
inline any_t string_to_any<unsigned long>(const std::string& data) {
    return any_t(std::stoul(data));
}

template <>
inline any_t string_to_any<unsigned long long>(const std::string& data) {
    return any_t(std::stoull(data));
}

template <>
inline any_t string_to_any<float>(const std::string& data) {
    return any_t(std::stof(data));
}

template <>
inline any_t string_to_any<double>(const std::string& data) {
    return any_t(std::stod(data));
}

template <>
inline any_t string_to_any<long double>(const std::string& data) {
    return any_t(std::stold(data));
}

}  // namespace hku
