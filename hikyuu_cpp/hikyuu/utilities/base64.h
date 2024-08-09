/*
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-6-2
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_BASE64_H
#define HKU_UTILS_BASE64_H

#include <memory>
#include "string_view.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**
 * 将二进制 bytes 数组编码成 base64 字符串
 * @param bytes_to_encode 内存起始地址
 * @param in_len 待计算的字节长度
 */
std::string HKU_UTILS_API base64_encode(unsigned char const* bytes_to_encode, size_t in_len);

/**
 * 字符串编码为 base64
 * @param bytes_to_encode 内存起始地址
 * @param in_len 待计算的字节长度
 * @note 通过 func(unsigned char *, unsigned int) 函数实现，而不是直接只提供 string_view
 * 版本的原因是：c++17 string_view 处理 nullptr 时，程序会直接挂掉，无异常
 */
inline std::string base64_encode(string_view src) {
    return base64_encode((unsigned char const*)src.data(), src.size());
}

/**
 * 将 base64 字符串解码
 * @param encoded_string base64 编码的字符串
 * @param in_len 字符串长度
 * @return string 实际解码后的二进制内容保存在返回的字符串对象中
 * @note 如果传入的base64编码字符串中含有非法字符，不会告警，仅处理到能处理的字符
 */
std::string HKU_UTILS_API base64_decode(unsigned char const* encoded_string, size_t in_len);

/**
 * 将 base64 字符串解码
 * @param encoded_string base64 编码的字符串
 * @return string 实际解码后的二进制内容保存在返回的字符串对象中
 */
inline std::string base64_decode(string_view encoded_string) {
    return base64_decode((unsigned char const*)encoded_string.data(), encoded_string.size());
}

}  // namespace hku

#endif  // HKU_UTILS_BASE64_H