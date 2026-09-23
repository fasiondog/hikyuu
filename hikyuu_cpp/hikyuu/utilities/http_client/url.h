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
 * @brief URL encoding (the percent encoding)
 *
 * Convert the special characters in the string into the URL safe percent encoded format.
 * The alphanumeric characters and - _ . ~ stay unchanged, the other characters are converted into
 * the %XX format.
 *
 * @param istr the input string to be encoded
 * @return std::string the encoded string
 *
 * @par Example
 * @code
 * url_escape("hello world")      // Returns "hello%20world"
 * url_unescape("hello%20world")  // Returns "hello world"
 * @endcode
 */
std::string HKU_UTILS_API url_escape(const char* istr);

/**
 * @brief URL decoding
 *
 * Convert the percent encoded string back to the original string.
 * Convert the encoding in the %XX format back to the corresponding character.
 *
 * @param istr the input string to be decoded
 * @return std::string the decoded original string
 *
 * @par Example
 * @code
 * url_unescape("hello%20world")  // Returns "hello world"
 * url_unescape("a%2Bb%3Dc")      // Returns "a+b=c"
 * @endcode
 */
std::string HKU_UTILS_API url_unescape(const char* istr);

}  // namespace hku

#endif