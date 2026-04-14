/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2026-04-10
 *      Author: Jet
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
 *
 * 算法说明：
 * - 使用贪心回溯算法，平均时间复杂度 O(m+n)，最坏 O(m*n)
 * - 空间复杂度 O(1)，仅使用常数个额外变量
 * - 相比 DP 方法，在实际场景中通常更快且内存占用更低
 */
inline bool wildcardMatchImpl(const std::string& text, const std::string& pattern) {
    size_t m = text.length();
    size_t n = pattern.length();

    size_t i = 0;                        // text 的当前索引
    size_t j = 0;                        // pattern 的当前索引
    size_t starIdx = std::string::npos;  // 最近一个 '*' 的位置
    size_t matchIdx = 0;                 // '*' 开始匹配 text 的位置

    while (i < m) {
        if (j < n && (pattern[j] == '?' || pattern[j] == text[i])) {
            // 当前字符匹配或遇到 '?'
            ++i;
            ++j;
        } else if (j < n && pattern[j] == '*') {
            // 遇到 '*'，记录位置并尝试匹配空字符串
            starIdx = j;
            matchIdx = i;
            ++j;  // 移动 pattern 指针，'*' 先尝试匹配空串
        } else if (starIdx != std::string::npos) {
            // 匹配失败且有 '*' 可以回溯
            // '*' 多匹配一个字符
            ++matchIdx;
            i = matchIdx;
            j = starIdx + 1;  // pattern 回到 '*' 之后的位置
        } else {
            // 匹配失败且无法回溯
            return false;
        }
    }

    // 处理 pattern 末尾剩余的 '*'
    while (j < n && pattern[j] == '*') {
        ++j;
    }

    return j == n;
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
    bool hasWildcard =
      (pattern.find('*') != std::string::npos) || (pattern.find('?') != std::string::npos);

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
