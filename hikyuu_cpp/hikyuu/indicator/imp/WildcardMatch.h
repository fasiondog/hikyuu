/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_WILDCARD_MATCH_H_
#define INDICATOR_IMP_WILDCARD_MATCH_H_

#include <string>
#include <vector>

namespace hku {

/**
 * @brief 内部通配符匹配函数，支持*和?
 * @param text 待匹配的文本
 * @param pattern 匹配模式，支持通配符*和?
 * @return 是否匹配成功
 * 
 * 通配符说明：
 * - * 匹配任意字符序列（包括空序列）
 * - ? 匹配任意单个字符
 */
inline bool wildcardMatchImpl(const std::string& text, const std::string& pattern) {
    size_t m = text.length();
    size_t n = pattern.length();

    // DP表，dp[i][j]表示text前i个字符是否匹配pattern前j个字符
    std::vector<std::vector<bool>> dp(m + 1, std::vector<bool>(n + 1, false));

    // 空字符串匹配空模式
    dp[0][0] = true;

    // 处理模式开头连续的*，它们可以匹配空字符串
    for (size_t j = 1; j <= n; ++j) {
        if (pattern[j - 1] == '*') {
            dp[0][j] = dp[0][j - 1];
        }
    }

    // 填充DP表
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (pattern[j - 1] == '*') {
                // *可以匹配任意字符序列（包括空序列）
                dp[i][j] = dp[i][j - 1]      // *匹配空序列
                        || dp[i - 1][j];     // *匹配至少一个字符
            } else if (pattern[j - 1] == '?' || pattern[j - 1] == text[i - 1]) {
                // ?匹配任意单个字符，或者字符精确匹配
                dp[i][j] = dp[i - 1][j - 1];
            }
        }
    }

    return dp[m][n];
}

/**
 * @brief 通配符匹配函数，支持*和?
 * @param text 待匹配的文本
 * @param pattern 匹配模式，支持通配符*和?
 * @return 是否匹配成功
 * 
 * 通配符说明：
 * - * 匹配任意字符序列（包括空序列）
 * - ? 匹配任意单个字符
 * 
 * 特殊规则：
 * - 如果pattern中不包含通配符(*或?)，则自动进行开头匹配（等价于pattern*）
 * - 如果pattern中包含通配符，则进行完全匹配
 */
inline bool wildcardMatch(const std::string& text, const std::string& pattern) {
    // 检查pattern中是否包含通配符
    bool hasWildcard = (pattern.find('*') != std::string::npos) || 
                       (pattern.find('?') != std::string::npos);
    
    if (hasWildcard) {
        // 包含通配符，使用完全匹配
        return wildcardMatchImpl(text, pattern);
    } else {
        // 不包含通配符，使用开头匹配（等价于pattern*）
        return text.starts_with(pattern);
    }
}

} /* namespace hku */

#endif /* INDICATOR_IMP_WILDCARD_MATCH_H_ */
