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
    if (utf8_str.empty() || out_len == 0) {
        return 0;
    }
    utf8proc_ssize_t result = utf8proc_decompose(
      (const utf8proc_uint8_t*)utf8_str.data(), (utf8proc_ssize_t)utf8_str.size(), out,
      (utf8proc_ssize_t)out_len, static_cast<utf8proc_option_t>(0));
    if (result < 0) {
        memset(out, 0, out_len * sizeof(int32_t));
        return 0;
    }
    return ((size_t)result > out_len) ? out_len : (size_t)result;
}

}  // namespace hku
