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
#else
#include <iconv.h>
#include <string.h>
#include <iostream>
#include <codecvt>
#include <locale>
#endif

namespace hku {

template double HKU_UTILS_API roundEx(double number, int ndigits);
template float HKU_UTILS_API roundEx(float number, int ndigits);
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

std::string HKU_UTILS_API utf8_to_utf32(const std::string &utf8_str, size_t max_len) {
    // 步骤 1：UTF-8 → UTF-16（Windows 原生宽字符）
    int utf16_len = MultiByteToWideChar(CP_UTF8,           // 源编码：UTF-8
                                        0,                 // 转换标志
                                        utf8_str.c_str(),  // 输入 UTF-8 字符串
                                        -1,                // 自动计算长度（包含空终止符）
                                        nullptr,           // 输出缓冲区（先获取长度）
                                        0                  // 输出缓冲区大小
    );
    if (utf16_len == 0) {
        throw std::runtime_error("Failed convert UTF-8 to UTF-16!");
    }

    std::vector<wchar_t> utf16_buf(utf16_len);
    if (!MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, utf16_buf.data(), utf16_len)) {
        throw std::runtime_error("Failed convert UTF-8 to UTF-16!");
    }

    // 步骤 2：UTF-16 → UTF-32（与 NumPy U 类型匹配）
    std::vector<uint32_t> utf32_chars;
    for (size_t i = 0; i < utf16_len - 1; ++i) {  // 排除最后的空终止符
        wchar_t c = utf16_buf[i];
        if (c >= 0xD800 && c <= 0xDBFF) {  // 高代理项
            if (i + 1 < utf16_len - 1) {
                wchar_t c2 = utf16_buf[i + 1];
                if (c2 >= 0xDC00 && c2 <= 0xDFFF) {  // 低代理项
                    // 组合为 UTF-32 字符
                    uint32_t code = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
                    utf32_chars.push_back(code);
                    i++;  // 跳过已处理的低代理项
                    continue;
                }
            }
        }
        // 普通字符直接转换
        utf32_chars.push_back(static_cast<uint32_t>(c));
    }

    // 步骤 3：截断或填充到 max_len 个字符（每个字符 4 字节）
    if (utf32_chars.size() > max_len) {
        utf32_chars.resize(max_len);
    } else {
        utf32_chars.resize(max_len, 0);  // 不足补 0（对应 L'\0'）
    }

    // 步骤 4：将 UTF-32 字符数组转为 std::string（存储原始字节）
    return std::string(reinterpret_cast<const char *>(utf32_chars.data()),
                       utf32_chars.size() * sizeof(uint32_t));
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

std::string HKU_UTILS_API utf8_to_utf32(const std::string &str, size_t max_len) {
    // 转换 UTF-8 到 UTF-32（wstring 存储）
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::wstring wstr = converter.from_bytes(str);

    // 截断或填充到指定长度（max_len 个字符）
    wstr.resize(max_len, L'\0');  // 不足补空字符

    // 将 wchar_t 数组的字节直接复制到 std::string 中
    // 注意：std::string 在这里仅作为字节容器，不表示字符含义
    return std::string(reinterpret_cast<const char *>(wstr.data()), wstr.size() * sizeof(wchar_t));
}

#endif /* defined(_MSC_VER) */

// 将UTF-8字符串转换为UTF-32码点
size_t HKU_UTILS_API utf8_to_utf32(const std::string &utf8_str, int32_t *out, size_t out_len) {
    // 检查输出缓冲区有效性
    if (out == nullptr || out_len == 0) {
        return 0;
    }

    const uint8_t *input = reinterpret_cast<const uint8_t *>(utf8_str.data());
    size_t input_len = utf8_str.size();
    size_t pos = 0;
    size_t converted = 0;

    // 循环处理每个UTF-8码点
    while (pos < input_len && converted < out_len) {
        int32_t codepoint;
        ssize_t bytes_processed = utf8proc_iterate(input + pos, input_len - pos, &codepoint);

        if (bytes_processed < 0) {
            throw std::runtime_error("Invalid UTF-8 sequence at position " + std::to_string(pos));
        }
        if (bytes_processed == 0) {
            break;  // 正常结束
        }

        // 存储转换后的码点并更新计数
        out[converted++] = codepoint;
        pos += bytes_processed;
    }

    return converted;
}

}  // namespace hku
