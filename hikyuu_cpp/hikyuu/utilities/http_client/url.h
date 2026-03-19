/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_URL_H
#define HKU_UTILS_URL_H

#include <string>

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**
 * @brief URL 编码（百分号编码）
 *
 * 将字符串中的特殊字符转换为 URL 安全的百分号编码格式。
 * 字母数字以及 - _ . ~ 字符保持不变，其他字符转换为 %XX 格式。
 *
 * @param istr 待编码的输入字符串
 * @return std::string 编码后的字符串
 *
 * @par 示例
 * @code
 * url_escape("hello world")      // 返回 "hello%20world"
 * url_unescape("hello%20world")  // 返回 "hello world"
 * @endcode
 */
std::string HKU_UTILS_API url_escape(const char* istr);

/**
 * @brief URL 解码
 *
 * 将百分号编码的字符串转换回原始字符串。
 * 将 %XX 格式的编码转换回对应的字符。
 *
 * @param istr 待解码的输入字符串
 * @return std::string 解码后的原始字符串
 *
 * @par 示例
 * @code
 * url_unescape("hello%20world")  // 返回 "hello world"
 * url_unescape("a%2Bb%3Dc")      // 返回 "a+b=c"
 * @endcode
 */
std::string HKU_UTILS_API url_unescape(const char* istr);

}  // namespace hku

#endif