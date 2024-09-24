//
//  base64 encoding and decoding with C++.
//  Version: 2.rc.08 (release candidate)
//

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
 * 将 base64 字符串解码
 * @param encoded_string base64 编码的字符串
 * @param remove_linebreaks 是否移除url中的分隔符
 * @return string 实际解码后的二进制内容保存在返回的字符串对象中
 * @note 如果传入的base64编码字符串中含有非法字符，不会告警，仅处理到能处理的字符
 */
std::string HKU_UTILS_API base64_decode(std::string const& encoded_string,
                                        bool remove_linebreaks = false);

/**
 * 将二进制 bytes 数组编码成 base64 字符串
 * @param bytes_to_encode 内存起始地址
 * @param in_len 待计算的字节长度
 * @param url 是否为在 url 中使用
 */
std::string HKU_UTILS_API base64_encode(unsigned char const* bytes_to_encode, size_t in_len,
                                        bool url = false);

std::string HKU_UTILS_API base64_encode(const std::string& s, bool url = false);
std::string HKU_UTILS_API base64_encode_pem(const std::string& s);
std::string HKU_UTILS_API base64_encode_mime(const std::string& s);

#if __cplusplus >= 201703L
std::string HKU_UTILS_API base64_encode(string_view s, bool url = false);
std::string HKU_UTILS_API base64_encode_pem(string_view s);
std::string HKU_UTILS_API base64_encode_mime(string_view s);

std::string HKU_UTILS_API base64_decode(string_view s, bool remove_linebreaks = false);
#endif  // __cplusplus >= 201703L

}  // namespace hku

#endif  // HKU_UTILS_BASE64_H