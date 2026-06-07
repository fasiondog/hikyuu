/*
 * util.cpp
 *
 *  Created on: 2009-11-20
 *      Author: fasiondog
 */

#include <stdexcept>
#include <utf8proc.h>
#include "arithmetic.h"

#if defined(_MSC_VER)
#include <windows.h>
#include <iostream>
#else
#include <iconv.h>
#include <string.h>
#include <iostream>
#endif

namespace hku {

template double HKU_UTILS_API roundEx(double number, int ndigits);

template <>
float roundEx(float number, int ndigits) {
    // 国内一般使用传统四舍五入法
    if (ndigits < 0)
        return number;  // 无效位数直接返回原值

    const float factor = std::pow(10.0, ndigits);
    const float epsilon = 1e-10 * factor;  // 动态调整epsilon避免精度误差

    if (number >= 0)
        return std::floor(number * factor + 0.5 + epsilon) / factor;
    else
        return std::ceil(number * factor - 0.5 - epsilon) / factor;
}

template double HKU_UTILS_API roundUp(double number, int ndigits);
template float HKU_UTILS_API roundUp(float number, int ndigits);
template double HKU_UTILS_API roundDown(double number, int ndigits);
template float HKU_UTILS_API roundDown(float number, int ndigits);

#if defined(_MSC_VER)
/**
 * 将UTF8编码的字符串转换为GB2312编码的字符串
 * @param szinput 待转换的原始UTF8编码的字符串
 * @return 以GB2312编码的字符串
 * @note 仅在Windows平台下生效
 */
std::string HKU_UTILS_API utf8_to_gb(const char *szinput) {
    wchar_t *strSrc;
    char *szRes;
    std::string nullStr;
    if (!szinput) {
        return nullStr;
    }

    int i = MultiByteToWideChar(CP_UTF8, 0, szinput, -1, NULL, 0);
    if (i == 0) {
        return nullStr;
    }
    strSrc = new wchar_t[i + 1];
    if (!MultiByteToWideChar(CP_UTF8, 0, szinput, -1, strSrc, i)) {
        delete[] strSrc;
        return nullStr;
    }

    i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
    if (i == 0) {
        return nullStr;
    }

    szRes = new char[i + 1];
    if (!WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL)) {
        delete[] szRes;
        return nullStr;
    }

    std::string result(szRes);

    delete[] strSrc;
    delete[] szRes;
    return result;
}

std::string HKU_UTILS_API utf8_to_gb(const std::string &szinput) {
    return utf8_to_gb(szinput.c_str());
}

/**
 * 将GB2312编码的字符串转换为UTF8编码的字符串
 * @param szinput 待转换的原始GB2312编码的字符串
 * @return 以UTF8编码的字符串
 * @note 仅在Windows平台下生效
 */
std::string HKU_UTILS_API gb_to_utf8(const char *szinput) {
    wchar_t *strSrc;
    char *szRes;
    std::string nullstr;
    if (!szinput) {
        return nullstr;
    }

    int i = MultiByteToWideChar(CP_ACP, 0, szinput, -1, NULL, 0);
    if (0 == i) {
        return nullstr;
    }

    strSrc = new wchar_t[i + 1];
    if (!MultiByteToWideChar(CP_ACP, 0, szinput, -1, strSrc, i)) {
        delete[] strSrc;
        return nullstr;
    }

    i = WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, NULL, 0, NULL, NULL);
    if (0 == i) {
        return nullstr;
    }

    szRes = new char[i + 1];
    if (!WideCharToMultiByte(CP_UTF8, 0, strSrc, -1, szRes, i, NULL, NULL)) {
        delete[] szRes;
        return nullstr;
    }

    std::string result(szRes);

    delete[] strSrc;
    delete[] szRes;
    return result;
}

std::string HKU_UTILS_API gb_to_utf8(const std::string &szinput) {
    return gb_to_utf8(szinput.c_str());
}

#else /* else for defined(_MSC_VER) */
std::string HKU_UTILS_API utf8_to_gb(const std::string &szinput) {
    char *inbuf = const_cast<char *>(szinput.c_str());
    size_t inlen = strlen(inbuf);
    size_t outlen = inlen;
    char *outbuf = (char *)malloc(outlen);
    if (!outbuf) [[unlikely]] {
        return std::string();
    }
    memset(outbuf, 0, outlen);
    char *in = inbuf;
    char *out = outbuf;
    iconv_t cd = iconv_open("gbk", "utf-8");
    iconv(cd, &in, &inlen, &out, &outlen);
    iconv_close(cd);
    std::string result(outbuf);
    free(outbuf);
    return result;
}

std::string HKU_UTILS_API gb_to_utf8(const std::string &szinput) {
    char *inbuf = const_cast<char *>(szinput.c_str());
    size_t inlen = strlen(inbuf);
    size_t outlen = inlen * 2;
    char *outbuf = (char *)malloc(outlen);
    if (!outbuf) [[unlikely]] {
        return std::string();
    }
    memset(outbuf, 0, outlen);
    char *in = inbuf;
    char *out = outbuf;
    iconv_t cd = iconv_open("utf-8", "gbk");
    iconv(cd, &in, &inlen, &out, &outlen);
    iconv_close(cd);
    std::string result(outbuf);
    free(outbuf);
    return result;
}

#endif /* defined(_MSC_VER) */

bool HKU_UTILS_API isInteger(double num) {
    // 处理特殊值：NaN（非数值）或无穷大
    if (std::isnan(num) || std::isinf(num)) {
        return false;
    }

    // 计算小数部分（对 1.0 取模）
    double fractionalPart = num - std::floor(num);  // 等价于 num % 1.0，但处理负数更稳妥

    // 允许微小误差（因浮点数精度问题，如 5.0000000001 应视为 5）
    const double epsilon = 1e-9;
    return std::fabs(fractionalPart) < epsilon || std::fabs(fractionalPart - 1.0) < epsilon;
}

bool HKU_UTILS_API isInteger(float num) {
    // 处理特殊值：NaN（非数值）或无穷大（使用float专用函数）
    if (std::isnan(num) || std::isinf(num)) {
        return false;
    }

    // 计算小数部分（使用float专用的floor函数）
    float fractionalPart = num - std::floor(num);

    // 调整适合float的精度误差（float精度低于double，epsilon需更大）
    const float epsilon = 1e-5f;  // float典型精度范围内的误差阈值
    // 判断小数部分是否接近0或1.0（处理类似5.9999997这种情况）
    return std::fabs(fractionalPart) < epsilon || std::fabs(fractionalPart - 1.0f) < epsilon;
}

template double HKU_UTILS_API get_quantile(const std::vector<double> &vec, double quantile);
template float HKU_UTILS_API get_quantile(const std::vector<float> &vec, double quantile);

HKU_UTILS_API std::ostream &operator<<(std::ostream &os, const std::vector<double> &p) {
    if (p.empty()) {
        os << "[]";
        return os;
    }

    size_t len = p.size();
    const size_t print = 3;
    os << "[";
    for (size_t i = 0; i < len; i++) {
        if ((i < print) || (i + print >= len)) {
            os << p[i];
            if (i + 1 != len)
                os << ", ";
        } else if (i == 3)
            os << "..., ";
    }
    os << "]";
    return os;
}

HKU_UTILS_API std::ostream &operator<<(std::ostream &os, const std::vector<float> &p) {
    if (p.empty()) {
        os << "[]";
        return os;
    }

    size_t len = p.size();
    const size_t print = 3;
    os << "[";
    for (size_t i = 0; i < len; i++) {
        if ((i < print) || (i + print >= len)) {
            os << p[i];
            if (i + 1 != len)
                os << ", ";
        } else if (i == 3)
            os << "..., ";
    }
    os << "]";
    return os;
}

std::string HKU_UTILS_API utf8_to_lower(const std::string &s) noexcept {
    if (s.empty()) {
        return s;
    }
    std::string result;
    result.reserve(s.size());
    const uint8_t *str = reinterpret_cast<const uint8_t *>(s.data());
    utf8proc_ssize_t len = s.size();
    utf8proc_int32_t codepoint;
    utf8proc_ssize_t pos = 0;
    while ((pos = utf8proc_iterate(str, len, &codepoint)) > 0) {
        utf8proc_int32_t lower = utf8proc_tolower(codepoint);
        uint8_t buf[4];
        utf8proc_ssize_t encoded_len = utf8proc_encode_char(lower, buf);
        if (encoded_len > 0) {
            result.append(reinterpret_cast<const char *>(buf), encoded_len);
        }
        str += pos;
        len -= pos;
    }
    return result;
}

std::string HKU_UTILS_API utf8_to_upper(const std::string &s) noexcept {
    if (s.empty()) {
        return s;
    }
    std::string result;
    result.reserve(s.size());
    const uint8_t *str = reinterpret_cast<const uint8_t *>(s.data());
    utf8proc_ssize_t len = s.size();
    utf8proc_int32_t codepoint;
    utf8proc_ssize_t pos = 0;
    while ((pos = utf8proc_iterate(str, len, &codepoint)) > 0) {
        utf8proc_int32_t upper = utf8proc_toupper(codepoint);
        uint8_t buf[4];
        utf8proc_ssize_t encoded_len = utf8proc_encode_char(upper, buf);
        if (encoded_len > 0) {
            result.append(reinterpret_cast<const char *>(buf), encoded_len);
        }
        str += pos;
        len -= pos;
    }
    return result;
}

/* UTF-8字符串大小写折叠比较 */
bool HKU_UTILS_API utf8_fold_equal(const std::string &s1, const std::string &s2) noexcept {
    if (s1.empty() && s2.empty()) {
        return true;
    }
    if (s1.empty() || s2.empty()) {
        return false;
    }

    const uint8_t *str1 = reinterpret_cast<const uint8_t *>(s1.data());
    utf8proc_ssize_t len1 = s1.size();
    const uint8_t *str2 = reinterpret_cast<const uint8_t *>(s2.data());
    utf8proc_ssize_t len2 = s2.size();

    utf8proc_int32_t codepoint1, codepoint2;
    utf8proc_ssize_t pos1 = 0, pos2 = 0;

    while (true) {
        pos1 = utf8proc_iterate(str1, len1, &codepoint1);
        pos2 = utf8proc_iterate(str2, len2, &codepoint2);

        if (pos1 <= 0 && pos2 <= 0) {
            return true;
        }
        if (pos1 <= 0 || pos2 <= 0) {
            return false;
        }

        utf8proc_int32_t upper1 = utf8proc_toupper(codepoint1);
        utf8proc_int32_t upper2 = utf8proc_toupper(codepoint2);

        if (upper1 != upper2) {
            return false;
        }

        str1 += pos1;
        len1 -= pos1;
        str2 += pos2;
        len2 -= pos2;
    }
}

/* UTF-8字符串包含子字符串 */
bool HKU_UTILS_API utf8_contains(const std::string &s, const std::string &sub) noexcept {
    if (sub.empty()) {
        return true;
    }
    if (s.empty()) {
        return false;
    }

    const uint8_t *str = reinterpret_cast<const uint8_t *>(s.data());
    utf8proc_ssize_t len = s.size();
    const uint8_t *sub_str = reinterpret_cast<const uint8_t *>(sub.data());
    utf8proc_ssize_t sub_len = sub.size();

    utf8proc_int32_t codepoint;
    utf8proc_ssize_t pos = 0;

    while ((pos = utf8proc_iterate(str, len, &codepoint)) > 0) {
        const uint8_t *tmp_str = str;
        utf8proc_ssize_t tmp_len = len;
        const uint8_t *tmp_sub = sub_str;
        utf8proc_ssize_t tmp_sub_len = sub_len;

        utf8proc_int32_t cp1, cp2;
        utf8proc_ssize_t p1, p2;

        bool match = true;
        while ((p2 = utf8proc_iterate(tmp_sub, tmp_sub_len, &cp2)) > 0) {
            p1 = utf8proc_iterate(tmp_str, tmp_len, &cp1);
            if (p1 <= 0 || cp1 != cp2) {
                match = false;
                break;
            }
            tmp_str += p1;
            tmp_len -= p1;
            tmp_sub += p2;
            tmp_sub_len -= p2;
        }

        if (match) {
            return true;
        }

        str += pos;
        len -= pos;
    }

    return false;
}

}  // namespace hku