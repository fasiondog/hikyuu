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
#include <utf8proc.h>

namespace hku {

inline void utf8_to_unicode(const std::string& utf8_str, std::vector<int32_t>& out) {
    out.clear();
    if (utf8_str.empty()) {
        return;
    }

    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(utf8_str.data());
    size_t len = utf8_str.size();

    while (len > 0) {
        int32_t codepoint;
        size_t bytes = utf8proc_iterate(ptr, len, &codepoint);
        if (bytes == 0) {
            break;
        }
        out.push_back(codepoint);
        ptr += bytes;
        len -= bytes;
    }
}

inline bool wildcardMatchImpl(const std::vector<int32_t>& text,
                              const std::vector<int32_t>& pattern) {
    size_t m = text.size();
    size_t n = pattern.size();

    size_t i = 0;
    size_t j = 0;
    size_t starIdx = std::string::npos;
    size_t matchIdx = 0;

    while (i < m) {
        if (j < n && (pattern[j] == '?' || pattern[j] == text[i])) {
            ++i;
            ++j;
        } else if (j < n && pattern[j] == '*') {
            starIdx = j;
            matchIdx = i;
            ++j;
        } else if (starIdx != std::string::npos) {
            ++matchIdx;
            i = matchIdx;
            j = starIdx + 1;
        } else {
            return false;
        }
    }

    while (j < n && pattern[j] == '*') {
        ++j;
    }

    return j == n;
}

inline bool wildcardMatch(const std::string& text, const std::string& pattern) {
    bool hasWildcard =
      (pattern.find('*') != std::string::npos) || (pattern.find('?') != std::string::npos);

    if (hasWildcard) {
        std::vector<int32_t> text_unicode;
        std::vector<int32_t> pattern_unicode;
        utf8_to_unicode(text, text_unicode);
        utf8_to_unicode(pattern, pattern_unicode);
        return wildcardMatchImpl(text_unicode, pattern_unicode);
    } else {
        std::vector<int32_t> text_unicode;
        std::vector<int32_t> pattern_unicode;
        utf8_to_unicode(text, text_unicode);
        utf8_to_unicode(pattern, pattern_unicode);

        size_t patternLen = pattern_unicode.size();
        size_t textLen = text_unicode.size();

        if (patternLen > textLen) {
            return false;
        }

        for (size_t i = 0; i < patternLen; ++i) {
            if (text_unicode[i] != pattern_unicode[i]) {
                return false;
            }
        }
        return true;
    }
}

} /* namespace hku */

#endif /* INDICATOR_IMP_WILDCARD_MATCH_H_ */