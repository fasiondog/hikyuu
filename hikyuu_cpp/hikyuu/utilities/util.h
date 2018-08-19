/*
 * util.h
 *
 *  Created on: 2009-11-20
 *      Author: fasiondog
 */

#ifndef UTIL_H_
#define UTIL_H_

#ifndef DATATYPE_H_
#include <string>
#include <boost/config.hpp>

#ifndef HKU_API
#define HKU_API
#endif

#endif /* DATATYPE_H_ */

namespace hku {

#ifndef DATATYPE_H_
using std::string;
#endif

/**
 * @ingroup Common-Utilities
 * @{
 */

#if defined(BOOST_WINDOWS)
string HKU_API utf8_to_gb(const char* szinput);
string HKU_API utf8_to_gb(const string& szinput);
string HKU_API gb_to_utf8(const char* szinput);
string HKU_API gb_to_utf8(const string& szinput);
#else
string HKU_API utf8_to_gb(const string& szinput);
string HKU_API gb_to_utf8(const string& szinput);
#endif

/**
 * Windows平台下将字符串由UTF8转换为GB2312编码，
 * Linux平台下不做任何事
 * @note 为跨平台，源代码文件必须使用UTF8编码保存，程序中出现的
 *       中文字符串，应使用HKU_STR，如：std::cout << HKU_STR("中国")
 */
#if defined(BOOST_WINDOWS) && (PY_VERSION_HEX < 0x03000000)
    //将utf8编码的字符串转换为GB2312编码
    #define UTF8ToGB utf8_to_gb
    #define GBToUTF8 gb_to_utf8
    #define HKU_STR(s) UTF8ToGB(s)
#else
    #define HKU_STR(s) (s)
    #define GBToUTF8(s) (s)
    #define UTF8ToGB(s) (s)
#endif


/**
 * 四舍五入，和python中的round行为一样
 * @param number  待四舍五入的数据
 * @param ndigits 保留小数位数
 * @return 处理过的数据
 */
double HKU_API roundEx(double number, int ndigits=0);

/**
 * 向上截取，如10.1截取后为11
 * @param number  待处理数据
 * @param ndigits 保留小数位数
 * @return 处理过的数据
 */
double HKU_API roundUp(double number, int ndigits=0);

/**
 * 向下截取，如10.1截取后为10
 * @param number  待处理数据
 * @param ndigits 保留小数位数
 * @return 处理过的数据
 */
double HKU_API roundDown(double number, int ndigits=0);


/** @} */
} /* namespace */

#endif /* UTIL_H_ */
