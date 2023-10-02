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
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>

namespace hku {

/**
 * @ingroup Utilities
 * @{
 */

/** 转小写字符串 */
inline void to_lower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
}

/** 转大写字符串 */
inline void to_upper(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
}

/** 删除字符串两端空格 */
inline void trim(std::string& s) {
    if (s.empty()) {
        return;
    }

    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
}

/**
 * 分割字符串
 * @param str 待封的字符串
 * @param c 分割符
 */
inline std::vector<std::string_view> split(const std::string& str, char c) {
    std::vector<std::string_view> result;
    std::string_view view(str);
    size_t prepos = 0;
    size_t pos = view.find_first_of(c);
    while (pos != std::string::npos) {
        result.emplace_back(view.substr(prepos, pos - prepos));
        prepos = pos + 1;
        pos = view.find_first_of(c, prepos);
    }

    if (prepos < str.size() - 1) {
        result.emplace_back(str.substr(prepos));
    }
    return result;
}

/**
 * 分割字符串
 * @param str 待分割的string_view
 * @param c 分割字符
 * @return string_view 组成的 vector
 * @note 注意返回结果的生命周期应小于输入的字符串相同！
 */
inline std::vector<std::string_view> split(const std::string_view& view, char c) {
    std::vector<std::string_view> result;
    size_t prepos = 0;
    size_t pos = view.find_first_of(c);
    while (pos != std::string::npos) {
        result.emplace_back(view.substr(prepos, pos - prepos));
        prepos = pos + 1;
        pos = view.find_first_of(c, prepos);
    }

    if (prepos < view.size() - 1) {
        result.emplace_back(view.substr(prepos));
    }
    return result;
}

inline std::vector<std::string_view> split(const std::string_view& str,
                                           const std::string& split_str) {
    std::vector<std::string_view> result;
    size_t split_str_len = split_str.size();
    if (split_str_len == 0) {
        result.emplace_back(str);
        return result;
    }

    size_t prepos = 0;
    size_t pos = str.find(split_str);
    while (pos != std::string::npos) {
        result.emplace_back(str.substr(prepos, pos - prepos));
        prepos = pos + split_str_len;
        pos = str.find(split_str, prepos);
    }

    if (prepos < str.size() - 1) {
        result.emplace_back(str.substr(prepos));
    }
    return result;
}

/** @} */
} /* namespace hku */

#endif /* HIKYUU_UTILITIES_ARITHMETIC_H */