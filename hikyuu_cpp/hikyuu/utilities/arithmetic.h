/*
 * arithmetic.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-7-15
 *      Author: fasiondog
 */

#pragma once
#ifndef HIKYUU_UTILITIES_ARITHMETIC_H
#define HIKYUU_UTILITIES_ARITHMETIC_H

#include <cctype>
#include <string>
#include <algorithm>

namespace hku {

/**
 * @ingroup Utilities
 * @{
 */

/** 转小写字符串 */
void to_lower(std::string& s);
inline void to_lower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
}

/** 转大写字符串 */
void to_upper(std::string& s);
inline void to_upper(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
}

/** 删除字符串两端空格 */
void trim(std::string& s);
inline void trim(std::string& s) {
    if (s.empty()) {
        return;
    }

    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
}

/** @} */
} /* namespace hku */

#endif /* HIKYUU_UTILITIES_ARITHMETIC_H */