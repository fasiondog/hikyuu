/*
 * arithmetic.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-7-15
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_ARITHMETIC_H
#define HIKYUU_UTILITIES_ARITHMETIC_H

#include <cctype>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

/**
 * @ingroup Utilities
 * @{
 */

#if defined(_MSC_VER)
std::string HKU_API utf8_to_gb(const char* szinput);
std::string HKU_API utf8_to_gb(const std::string& szinput);
std::string HKU_API gb_to_utf8(const char* szinput);
std::string HKU_API gb_to_utf8(const std::string& szinput);
#else
std::string HKU_API utf8_to_gb(const std::string& szinput);
std::string HKU_API gb_to_utf8(const std::string& szinput);
#endif

#define UTF8ToGB hku::utf8_to_gb
#define GBToUTF8 hku::gb_to_utf8

/**
 * Windows平台下用于将字符串由UTF8转换为GB2312编码（仅用于打印，如需用于路径，请使用 HKU_PATH），
 * Linux平台下不做任何事
 * @note 目前 Visual Studio 2019 已修正 UTF8 问题，大部分情况下无需使用，HKU_STR/HKU_CSTR，
 *       但windows的路径和文件名仍旧是GB，需要进行转换处理。
 */
#if defined(_MSC_VER) && _MSC_VER < 1928  // 1928 (Visual Studio 2019)
// 将utf8编码的字符串转换为GB2312编码
#define HKU_STR(s) UTF8ToGB(s)
#define HKU_CSTR(s) UTF8ToGB(s)
#else
#define HKU_STR(s) (s)
#define HKU_CSTR(s) (s)
#endif

/**
 * 用于处理路径文件名，兼容windows中文平台
 * HKU_PATH 适用于 std::string
 * HKU_CPATH 适用于 char *
 */
#if defined(_MSC_VER)
#define HKU_PATH(s) UTF8ToGB(s)
#define HKU_CPATH(s) UTF8ToGB(s)
#else
#define HKU_PATH(s) (s)
#define HKU_CPATH(s) (s)
#endif

/**
 * 四舍五入，ROUND_HALF_EVEN 银行家舍入法
 * @param number  待四舍五入的数据
 * @param ndigits 保留小数位数
 * @return 处理过的数据
 */
double HKU_API roundEx(double number, int ndigits = 0);

/**
 * 向上截取，如10.1截取后为11
 * @param number  待处理数据
 * @param ndigits 保留小数位数
 * @return 处理过的数据
 */
double HKU_API roundUp(double number, int ndigits = 0);

/**
 * 向下截取，如10.1截取后为10
 * @param number  待处理数据
 * @param ndigits 保留小数位数
 * @return 处理过的数据
 */
double HKU_API roundDown(double number, int ndigits = 0);

/** 转小写字符串 */
inline void to_lower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
}

/** 转大写字符串 */
inline void to_upper(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
}

/** 删除字符串两端空格 */
inline void trim(std::string& s) {
    if (s.empty()) {
        return;
    }

    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
}

/**
 * 分割字符串
 * @param str 待封的字符串
 * @param c 分割符
 */
inline std::vector<std::string_view> split(const std::string& str, char c) {
    std::vector<std::string_view> result;
    std::string_view view(str);
    size_t prepos = 0;
    size_t pos = view.find_first_of(c);
    while (pos != std::string::npos) {
        result.emplace_back(view.substr(prepos, pos - prepos));
        prepos = pos + 1;
        pos = view.find_first_of(c, prepos);
    }

    if (prepos < str.size() - 1) {
        result.emplace_back(str.substr(prepos));
    }
    return result;
}

/**
 * 分割字符串
 * @param str 待分割的string_view
 * @param c 分割字符
 * @return string_view 组成的 vector
 * @note 注意返回结果的生命周期应小于输入的字符串相同！
 */
inline std::vector<std::string_view> split(const std::string_view& view, char c) {
    std::vector<std::string_view> result;
    size_t prepos = 0;
    size_t pos = view.find_first_of(c);
    while (pos != std::string::npos) {
        result.emplace_back(view.substr(prepos, pos - prepos));
        prepos = pos + 1;
        pos = view.find_first_of(c, prepos);
    }

    if (prepos < view.size() - 1) {
        result.emplace_back(view.substr(prepos));
    }
    return result;
}

inline std::vector<std::string_view> split(const std::string_view& str,
                                           const std::string& split_str) {
    std::vector<std::string_view> result;
    size_t split_str_len = split_str.size();
    if (split_str_len == 0) {
        result.emplace_back(str);
        return result;
    }

    size_t prepos = 0;
    size_t pos = str.find(split_str);
    while (pos != std::string::npos) {
        result.emplace_back(str.substr(prepos, pos - prepos));
        prepos = pos + split_str_len;
        pos = str.find(split_str, prepos);
    }

    if (prepos < str.size() - 1) {
        result.emplace_back(str.substr(prepos));
    }
    return result;
}

/**
 * byte 转 16 进制字符串, 如 "abcd" 转换为 "61626364"
 * @param in_byte 输入的 byte 数组
 * @param in_len byte 数组长度
 */
inline std::string byteToHexStr(const char* bytes, size_t in_len) {
    std::string hexstr;
    const unsigned char* in_byte = (const unsigned char*)bytes;
    if (in_byte == nullptr) {
        return hexstr;
    }

    char* buf = new char[2 * in_len + 1];
    size_t buf_ix = 0;

    for (size_t i = 0; i < in_len; ++i) {
        unsigned char high = in_byte[i] >> 4;
        unsigned char low = in_byte[i] & 0x0F;

        buf[buf_ix++] = high <= 9 ? high + 0x30 : high + 87;
        buf[buf_ix++] = low <= 9 ? low + 0x30 : low + 87;
    }

    buf[buf_ix++] = '\0';
    hexstr = std::string(buf);
    delete[] buf;
    return hexstr;
}

/**
 * byte 转 16 进制字符串, 如 "abcd" 转换为 "61626364"
 * @param in_byte std::string 格式的输入
 */
inline std::string byteToHexStr(const std::string& bytes) {
    return byteToHexStr(bytes.c_str(), bytes.size());
}

/**
 * byte 转 16 进制字符串, 如 "abcd" 转换为 "0x61 0x62 0x63 0x64"
 * @param in_byte 输入的 byte 数组
 * @param in_len byte 数组长度
 */
inline std::string byteToHexStrForPrint(const char* bytes, size_t in_len) {
    std::string hexstr;
    const unsigned char* in_byte = (const unsigned char*)bytes;
    if (in_byte == nullptr) {
        return hexstr;
    }

    char* buf = new char[5 * in_len + 1];
    size_t buf_ix = 0;

    for (size_t i = 0; i < in_len; ++i) {
        buf[buf_ix++] = '0';
        buf[buf_ix++] = 'x';
        unsigned char high = in_byte[i] >> 4;
        unsigned char low = in_byte[i] & 0x0F;

        buf[buf_ix++] = high <= 9 ? high + 0x30 : high + 87;
        buf[buf_ix++] = low <= 9 ? low + 0x30 : low + 87;
        buf[buf_ix++] = ' ';
    }

    if (buf_ix == 0) {
        buf[buf_ix] = '\0';
    } else {
        buf[--buf_ix] = '\0';
    }
    hexstr = std::string(buf);
    delete[] buf;
    return hexstr;
}

/**
 * byte 转 16 进制字符串, 如 "abcd" 转换为 "61626364"
 * @param in_byte 输入的 byte 数组
 */
inline std::string byteToHexStrForPrint(const std::string& bytes) {
    return byteToHexStrForPrint(bytes.c_str(), bytes.size());
}

/** @} */
} /* namespace hku */

#endif /* HIKYUU_UTILITIES_ARITHMETIC_H */