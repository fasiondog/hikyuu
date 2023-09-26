/*
 * util.h
 *
 *  Created on: 2009-11-20
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILIIES_UTIL_H_
#define HIKYUU_UTILIIES_UTIL_H_

#include <string>

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

#define UTF8ToGB utf8_to_gb
#define GBToUTF8 gb_to_utf8

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

/** @} */
}  // namespace hku

#endif /* HIKYUU_UTILIIES_UTIL_H_ */
