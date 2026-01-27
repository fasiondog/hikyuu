/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-11
 *      Author: fasiondog
 */

#include <utf8proc.h>
#include "pybind_utils.h"

namespace hku {
size_t utf8_to_utf32(const std::string& utf8_str, int32_t* out, size_t out_len) noexcept {
    memset(out, 0, out_len * sizeof(int32_t));
    if (utf8_str.empty()) {
        return 0;
    }
    size_t result = utf8proc_decompose((const uint8_t*)utf8_str.data(), utf8_str.size(), out,
                                       out_len, UTF8PROC_NULLTERM);
    if (result < 0) {
        memset(out, 0, out_len * sizeof(int32_t));
        result = 0;
    }
    return result;
}

}  // namespace hku