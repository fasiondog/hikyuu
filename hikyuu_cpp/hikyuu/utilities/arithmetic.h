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

#include <cmath>
#include <cctype>
#include <vector>
#include <string>
#include <algorithm>

#include "string_view.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**
 * @ingroup Utilities
 * @{
 */

#if defined(_MSC_VER)
std::string HKU_UTILS_API utf8_to_gb(const char *szinput);
std::string HKU_UTILS_API utf8_to_gb(const std::string &szinput);
std::string HKU_UTILS_API gb_to_utf8(const char *szinput);
std::string HKU_UTILS_API gb_to_utf8(const std::string &szinput);
#else
std::string HKU_UTILS_API utf8_to_gb(const std::string &szinput);
std::string HKU_UTILS_API gb_to_utf8(const std::string &szinput);
#endif

#define UTF8ToGB hku::utf8_to_gb
#define GBToUTF8 hku::gb_to_utf8

/**
 * Under Windows it is used to convert a string from UTF8 to the GB2312 encoding (for the printing
 * only; use HKU_PATH if it is needed for a path),
 * Under Linux it does nothing
 * @note Currently Visual Studio 2019 has fixed the UTF8 problem, so it is not needed in most cases;
 * HKU_STR/HKU_CSTR,
 *       but the paths and file names under Windows are still GB and need the conversion processing.
 */
#if defined(_MSC_VER) && _MSC_VER < 1928  // 1928 (Visual Studio 2019)
// Convert a UTF8 encoded string to the GB2312 encoding
#define HKU_STR(s) UTF8ToGB(s)
#define HKU_CSTR(s) UTF8ToGB(s)
#else
#define HKU_STR(s) (s)
#define HKU_CSTR(s) (s)
#endif

/**
 * Used to process the path file names, compatible with the Chinese Windows platform
 * HKU_PATH is for std::string
 * HKU_CPATH is for char *
 */
#if defined(_MSC_VER)
#define HKU_PATH(s) UTF8ToGB(s)
#define HKU_CPATH(s) UTF8ToGB(s)
#else
#define HKU_PATH(s) (s)
#define HKU_CPATH(s) (s)
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

/**
 * Rounding, the ROUND_HALF_EVEN banker's rounding method
 * @param number  the data to be rounded
 * @param ndigits the number of the decimal places to keep
 * @return the processed data
 */
template <typename ValueT>
ValueT roundEx(ValueT number, int ndigits = 0) {
    // Switch to: ROUND_HALF_EVEN, the banker's rounding method
    // ValueT pow1, pow2, y, z;
    // ValueT x = number;
    // if (ndigits >= 0) {
    //     pow1 = pow(ValueT(10.0), ValueT(ndigits));
    //     pow2 = 1.0;
    //     y = (x * pow1) * pow2;
    // } else {
    //     pow1 = pow(ValueT(10.0), ValueT(-ndigits));
    //     pow2 = 1.0;
    //     y = x / pow1;
    // }

    // z = std::round(y);
    // if (std::fabs(y - z) == 0.5)
    //     /* halfway between two integers; use round-half-even */
    //     z = 2.0 * std::round(y / 2.0);

    // if (ndigits >= 0)
    //     z = (z / pow2) / pow1;
    // else
    //     z *= pow1;

    // In China the traditional rounding method is generally used
    if (ndigits < 0)
        return number;  // An invalid number of the digits returns the original value directly

    const double factor = std::pow(10.0, ndigits);
    const double epsilon =
      1e-10 * factor;  // Adjust epsilon dynamically to avoid the precision error

    if (number >= 0)
        return static_cast<ValueT>(std::floor(number * factor + 0.5 + epsilon) / factor);
    else
        return static_cast<ValueT>(std::ceil(number * factor - 0.5 - epsilon) / factor);
}

extern template double HKU_UTILS_API roundEx(double number, int ndigits);

template <>
float HKU_UTILS_API roundEx(float number, int ndigits);

/**
 * Truncate upward, e.g. 10.1 is truncated to 11
 * @param number  the data to be processed
 * @param ndigits the number of the decimal places to keep
 * @return the processed data
 */
template <typename ValueT>
ValueT roundUp(ValueT number, int ndigits = 0) {
    ValueT f;
    int i;
    f = 1.0;
    i = std::abs(ndigits);
    while (--i >= 0) {
        f = f * 10.0;
    }

    if (ndigits < 0) {
        number /= f;
    } else {
        number *= f;
    }

    if (number >= 0.0) {
        number = std::floor(number + 1.0);
    } else {
        number = std::ceil(number - 1.0);
    }

    if (ndigits < 0) {
        number *= f;
    } else {
        number /= f;
    }

    return number;
}

/**
 * Truncate downward, e.g. 10.1 is truncated to 10
 * @param number  the data to be processed
 * @param ndigits the number of the decimal places to keep
 * @return the processed data
 */
template <typename ValueT>
ValueT roundDown(ValueT number, int ndigits = 0) {
    ValueT f;
    int i;
    f = 1.0;
    i = std::abs(ndigits);
    while (--i >= 0) {
        f = f * 10.0;
    }

    if (ndigits < 0) {
        number /= f;
    } else {
        number *= f;
    }

    if (number >= 0.0) {
        number = std::floor(number);
    } else {
        number = std::ceil(number);
    }

    if (ndigits < 0) {
        number *= f;
    } else {
        number /= f;
    }

    return number;
}

extern template double HKU_UTILS_API roundUp(double number, int ndigits);
extern template float HKU_UTILS_API roundUp(float number, int ndigits);
extern template double HKU_UTILS_API roundDown(double number, int ndigits);
extern template float HKU_UTILS_API roundDown(float number, int ndigits);

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

/** Convert an ASCII string to lowercase */
inline void to_lower(std::string &s) noexcept {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
}

/** Convert a string to uppercase */
inline void to_upper(std::string &s) noexcept {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
}

/** Convert a UTF-8 string to lowercase */
std::string HKU_UTILS_API utf8_to_lower(const std::string &s) noexcept;

/** Convert a UTF-8 string to uppercase */
std::string HKU_UTILS_API utf8_to_upper(const std::string &s) noexcept;

/** Case folding comparison of UTF-8 strings */
bool HKU_UTILS_API utf8_fold_equal(const std::string &s1, const std::string &s2) noexcept;

/** Whether a UTF-8 string contains a substring */
bool HKU_UTILS_API utf8_contains(const std::string &s, const std::string &sub) noexcept;

/** Remove the spaces at both ends of a string */
inline void trim(std::string &s) {
    if (s.empty()) {
        return;
    }

    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    s.erase(s.find_last_not_of("\r") + 1);
    s.erase(s.find_last_not_of("\n") + 1);
}

#if CPP_STANDARD >= CPP_STANDARD_17
/**
 * Split a string
 * @param str the string to be split
 * @param c the separator
 */
inline std::vector<std::string_view> split(const std::string &str, char c) {
    std::vector<std::string_view> result;
    std::string_view view(str);
    size_t prepos = 0;
    size_t pos = view.find_first_of(c);
    while (pos != std::string::npos) {
        result.emplace_back(view.substr(prepos, pos - prepos));
        prepos = pos + 1;
        pos = view.find_first_of(c, prepos);
    }

    result.emplace_back(view.substr(prepos));
    return result;
}

/**
 * Split a string
 * @param str the string_view to be split
 * @param c the separator character
 * @return a vector composed of string_view
 * @note Note that the lifetime of the returned result should be shorter than that of the input
 *       string!
 */
inline std::vector<std::string_view> split(const std::string_view &view, char c) {
    std::vector<std::string_view> result;
    size_t prepos = 0;
    size_t pos = view.find_first_of(c);
    while (pos != std::string::npos) {
        result.emplace_back(view.substr(prepos, pos - prepos));
        prepos = pos + 1;
        pos = view.find_first_of(c, prepos);
    }

    result.emplace_back(view.substr(prepos));
    return result;
}

inline std::vector<std::string_view> split(const std::string_view &str,
                                           const std::string &split_str) {
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

    result.emplace_back(str.substr(prepos));
    return result;
}

#else
/**
 * Split a string
 * @param str the string to be split
 * @param c the separator
 */
inline std::vector<std::string> split(const std::string &str, char c) {
    std::vector<std::string> result;
    size_t prepos = 0;
    size_t pos = str.find_first_of(c);
    while (pos != std::string::npos) {
        result.emplace_back(str.substr(prepos, pos - prepos));
        prepos = pos + 1;
        pos = str.find_first_of(c, prepos);
    }

    result.emplace_back(str.substr(prepos));
    return result;
}

inline std::vector<std::string> split(const std::string &str, const std::string &split_str) {
    std::vector<std::string> result;
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

    result.emplace_back(str.substr(prepos));
    return result;
}
#endif /* #if CPP_STANDARD >= CPP_STANDARD_17 */

/**
 * Convert bytes to a hexadecimal string, e.g. "abcd" is converted to "61626364"
 * @param bytes the input byte array
 * @param in_len the length of the byte array
 */
inline std::string byteToHexStr(const char *bytes, size_t in_len) {
    std::string hexstr;
    const unsigned char *in_byte = (const unsigned char *)bytes;
    if (in_byte == nullptr) {
        return hexstr;
    }

    char *buf = new char[2 * in_len + 1];
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
 * Convert bytes to a hexadecimal string, e.g. "abcd" is converted to "61626364"
 * @param bytes the input in the std::string format
 */
inline std::string byteToHexStr(const std::string &bytes) {
    return byteToHexStr(bytes.c_str(), bytes.size());
}

/**
 * Convert bytes to a hexadecimal string, e.g. "abcd" is converted to "0x61 0x62 0x63 0x64"
 * @param bytes the input byte array
 * @param in_len the length of the byte array
 */
inline std::string byteToHexStrForPrint(const char *bytes, size_t in_len) {
    std::string hexstr;
    const unsigned char *in_byte = (const unsigned char *)bytes;
    if (in_byte == nullptr) {
        return hexstr;
    }

    char *buf = new char[5 * in_len + 1];
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
 * Convert bytes to a hexadecimal string, e.g. "abcd" is converted to "61626364"
 * @param bytes the input byte array
 */
inline std::string byteToHexStrForPrint(const std::string &bytes) {
    return byteToHexStrForPrint(bytes.c_str(), bytes.size());
}

/** Judge whether a double is an integer */
bool HKU_UTILS_API isInteger(double num);

/** Judge whether a float is an integer */
bool HKU_UTILS_API isInteger(float num);

/**
 * @brief Get the value of the given quantile in a sorted vector
 * @param vec the sorted array
 * @param quantile
 * @return Indicator::value_t
 */
template <typename T>
T get_quantile(const std::vector<T> &vec, double quantile) {
    if (vec.empty()) {
        return std::numeric_limits<T>::quiet_NaN();
    }

    if (quantile <= 0.0) {
        return vec.front();
    } else if (quantile >= 1.0) {
        return vec.back();
    }

    T ret;
    double qpos = vec.size() * quantile;
    if (qpos <= 1.0) {
        ret = vec[0];
    } else if (isInteger(qpos)) {
        size_t pos = static_cast<size_t>(qpos);
        ret = (vec[pos - 1] + vec[pos]) / 2;
    } else {
        size_t pos = static_cast<size_t>(qpos);
        T x = qpos - pos;
        ret = vec[pos - 1] + x * (vec[pos] - vec[pos - 1]);
    }
    return ret;
}

extern template double HKU_UTILS_API get_quantile(const std::vector<double> &vec, double quantile);
extern template float HKU_UTILS_API get_quantile(const std::vector<float> &vec, double quantile);

HKU_UTILS_API std::ostream &operator<<(std::ostream &os, const std::vector<double> &p);
HKU_UTILS_API std::ostream &operator<<(std::ostream &os, const std::vector<float> &p);

/** @} */
} /* namespace hku */

#endif /* HIKYUU_UTILITIES_ARITHMETIC_H */