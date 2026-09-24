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
 * Decode a base64 string
 * @param encoded_string the base64 encoded string
 * @param remove_linebreaks whether to remove the separators in the url
 * @return string the actually decoded binary content is stored in the returned string object
 * @note If the passed base64 encoded string contains illegal characters no warning is given, only
 *       the processable characters are processed
 */
std::string HKU_UTILS_API base64_decode(std::string const& encoded_string,
                                        bool remove_linebreaks = false);

/**
 * Encode a binary bytes array into a base64 string
 * @param bytes_to_encode the memory start address
 * @param in_len the number of the bytes to be calculated
 * @param url whether it is used in a url
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