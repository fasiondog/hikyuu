/*
 *  Copyright (c) 2019~2021, hikyuu
 *
 *  Created on: 2021/12/06
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_MD5_H
#define HKU_UTILS_MD5_H

#include <string>

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

/**
 * @brief 计算 md5 值
 *
 * @param input 待计算数据起始指针
 * @param len 待计算数据字节长度
 * @return std::string
 */
std::string HKU_UTILS_API md5(const unsigned char* input, size_t len);

/**
 * @brief 计算字符串 md5
 *
 * @param src 待计算的字符串
 * @return std::string
 */
inline std::string md5(const std::string& src) {
    return md5((const unsigned char*)src.data(), src.size());
}

}  // namespace hku

#endif  // #define HKU_UTILS_MD5_H