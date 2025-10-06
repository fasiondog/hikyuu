/*
 * util.cpp
 *
 *  Created on: 2009-11-20
 *      Author: fasiondog
 */

#include <stdexcept>
#include "arithmetic.h"

#if defined(_MSC_VER)
#include <windows.h>
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

}  // namespace hku
