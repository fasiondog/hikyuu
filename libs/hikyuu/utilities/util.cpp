/*
 * util.cpp
 *
 *  Created on: 2009-11-20
 *      Author: fasiondog
 */
#include <cmath>
#include "util.h"

#if defined(BOOST_WINDOWS)
#include <windows.h>
#endif

namespace hku {

double HKU_API roundEx(double number, int ndigits){
    double f;
    int i;
    f = 1.0;
    i = abs(ndigits);
    while  (--i >= 0){
        f = f*10.0;
    }

    if (ndigits < 0){
        number /= f;
    }else{
        number *= f;
    }

    if (number >= 0.0){
        number = std::floor(number + 0.5);
    }else{
        number = std::ceil(number - 0.5);
    }

    if (ndigits < 0){
        number *= f;
    }else{
        number /= f;
    }

    return number;
}

double HKU_API roundUp(double number, int ndigits) {
    double f;
    int i;
    f = 1.0;
    i = abs(ndigits);
    while  (--i >= 0){
        f = f*10.0;
    }

    if (ndigits < 0){
        number /= f;
    }else{
        number *= f;
    }

    if (number >= 0.0){
        number = std::floor(number + 1.0);
    }else{
        number = std::ceil(number - 1.0);
    }

    if (ndigits < 0){
        number *= f;
    }else{
        number /= f;
    }

    return number;
}

double HKU_API roundDown(double number, int ndigits) {
    double f;
    int i;
    f = 1.0;
    i = abs(ndigits);
    while  (--i >= 0){
        f = f*10.0;
    }

    if (ndigits < 0){
        number /= f;
    }else{
        number *= f;
    }

    if (number >= 0.0){
        number = std::floor(number);
    }else{
        number = std::ceil(number);
    }

    if (ndigits < 0){
        number *= f;
    }else{
        number /= f;
    }

    return number;
}


#if defined(BOOST_WINDOWS) && (PY_VERSION_HEX < 0x03000000)
/**
 * 将UTF8编码的字符串转换为GB2312编码的字符串
 * @param szinput 待转换的原始UTF8编码的字符串
 * @return 以GB2312编码的字符串
 * @note 仅在Windows平台下生效
 */
string HKU_API utf8_to_gb(const char* szinput) {
    wchar_t *strSrc;
    char *szRes;
    string nullStr;
    if (!szinput) {
        return nullStr;
    }

    int i = MultiByteToWideChar(CP_UTF8, 0, szinput, -1, NULL, 0);
    if (i == 0) {
        return nullStr;
    }
    strSrc = new wchar_t[i+1];    
    if (!MultiByteToWideChar(CP_UTF8, 0, szinput, -1, strSrc, i)) {
        delete[] strSrc;
        return nullStr;
    }

    i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
    if (i == 0) {
        return nullStr;
    }

    szRes = new char[i+1];
    if (!WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL)) {
        delete[] szRes;
        return nullStr;
    }
    
    string result(szRes);
    
    delete []strSrc;    
    delete []szRes;
    return result;
}

string HKU_API utf8_to_gb(const string& szinput) {
    return utf8_to_gb(szinput.c_str());
}

/**
 * 将GB2312编码的字符串转换为UTF8编码的字符串
 * @param szinput 待转换的原始GB2312编码的字符串
 * @return 以UTF8编码的字符串
 * @note 仅在Windows平台下生效
 */
string HKU_API gb_to_utf8(const char * szinput) {
    wchar_t *strSrc;
    char *szRes;
    string nullstr;
    if (!szinput) {
        return nullstr;
    }

    int i = MultiByteToWideChar(CP_ACP, 0, szinput, -1, NULL, 0);
    if (0 == i) {
        return nullstr;
    }

    strSrc = new wchar_t[i+1];
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

    string result(szRes);

    delete []strSrc;
    delete []szRes;
    return result;
}

string HKU_API gb_to_utf8(const string& szinput) {
    return gb_to_utf8(szinput.c_str());
}

#endif /* defined(BOOST_WINDOWS) */

} /*namespace*/
