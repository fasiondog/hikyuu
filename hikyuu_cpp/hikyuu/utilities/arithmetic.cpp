/*
 * util.cpp
 *
 *  Created on: 2009-11-20
 *      Author: fasiondog
 */

#include "arithmetic.h"

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <iconv.h>
#include <string.h>
#include <iostream>
#endif

namespace hku {

// double HKU_API roundEx(double number, int ndigits) {
//     // 切换至：ROUND_HALF_EVEN 银行家舍入法
//     double pow1, pow2, y, z;
//     double x = number;
//     if (ndigits >= 0) {
//         pow1 = pow(10.0, (double)ndigits);
//         pow2 = 1.0;
//         y = (x * pow1) * pow2;
//     } else {
//         pow1 = pow(10.0, (double)-ndigits);
//         pow2 = 1.0;
//         y = x / pow1;
//     }

//     z = round(y);
//     if (fabs(y - z) == 0.5)
//         /* halfway between two integers; use round-half-even */
//         z = 2.0 * round(y / 2.0);

//     if (ndigits >= 0)
//         z = (z / pow2) / pow1;
//     else
//         z *= pow1;

//     return z;
// }

double HKU_API roundUp(double number, int ndigits) {
    double f;
    int i;
    f = 1.0;
    i = abs(ndigits);
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

double HKU_API roundDown(double number, int ndigits) {
    double f;
    int i;
    f = 1.0;
    i = abs(ndigits);
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

#if defined(_MSC_VER)
/**
 * 将UTF8编码的字符串转换为GB2312编码的字符串
 * @param szinput 待转换的原始UTF8编码的字符串
 * @return 以GB2312编码的字符串
 * @note 仅在Windows平台下生效
 */
std::string HKU_API utf8_to_gb(const char *szinput) {
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

std::string HKU_API utf8_to_gb(const std::string &szinput) {
    return utf8_to_gb(szinput.c_str());
}

/**
 * 将GB2312编码的字符串转换为UTF8编码的字符串
 * @param szinput 待转换的原始GB2312编码的字符串
 * @return 以UTF8编码的字符串
 * @note 仅在Windows平台下生效
 */
std::string HKU_API gb_to_utf8(const char *szinput) {
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

std::string HKU_API gb_to_utf8(const std::string &szinput) {
    return gb_to_utf8(szinput.c_str());
}

#else /* else for defined(_MSC_VER) */
std::string HKU_API utf8_to_gb(const std::string &szinput) {
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

std::string HKU_API gb_to_utf8(const std::string &szinput) {
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

}  // namespace hku
