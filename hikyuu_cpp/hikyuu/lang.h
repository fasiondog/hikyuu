/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-09
 *      Author: fasiondog
 */

#pragma once

#include <string>
#include "hikyuu/utilities/cppdef.h"

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

// 注意仅在 hikyuu 相关项目内部使用，避免对项目造成污染

void loadLocalLanguage(const std::string &path = "i8n");

std::string HKU_API lang_htr(const char *id);

// 按上下文获取翻译
std::string HKU_API lang_hctr(const char *ctx, const char *id);

template <typename... Args>
std::string htr(const char *key, Args &&...args) {
    std::string fmt_str = lang_htr(key);
    return fmt::vformat(fmt_str, fmt::make_format_args(std::forward<Args>(args)...));
}

template <typename... Args>
std::string chtr(const char *ctx, const char *key, Args &&...args) {
    std::string fmt_str = lang_hctr(ctx, key);
    return fmt::vformat(fmt_str, fmt::make_format_args(std::forward<Args>(args)...));
}

}  // namespace hku