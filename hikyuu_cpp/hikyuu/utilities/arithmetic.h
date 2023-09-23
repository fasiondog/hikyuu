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

    result.emplace_back(view.substr(prepos));
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

    result.emplace_back(view.substr(prepos));
    return result;
}

/**
 * 获取指定序列索引的组合，返回由序列下标索引组成的组合集合
 * @note 组合的总数是 2**n - 1 个，所以inputs的长度需要控制
 * @exception 输入序列的最大长度仅支持15，超过时将抛出异常 HKUException
 * @param inputs 任意类型的序列
 * @return
 */
template <class T>
std::vector<std::vector<size_t>> combinateIndex(const std::vector<T>& inputs) {
    size_t total = inputs.size();
    // HKU_CHECK(total <= 15, "The length of inputs exceeds the maximum limit!");
    std::vector<std::vector<size_t>> result;

    std::vector<size_t> current;
    for (size_t i = 0; i < total; i++) {
        for (size_t j = 0, len = result.size(); j < len; j++) {
            current.resize(result[j].size());
            std::copy(result[j].cbegin(), result[j].cend(), current.begin());
            current.push_back(i);
            result.push_back(std::move(current));
        }
        current.clear();
        current.push_back(i);
        result.push_back(std::move(current));
    }

    return result;
}

/** @} */
} /* namespace hku */

#endif /* HIKYUU_UTILITIES_ARITHMETIC_H */