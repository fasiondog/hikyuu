/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-09
 *      Author: fasiondog
 */

#pragma once

#include <string>

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

// 注意仅在 hikyuu 相关项目内部使用，避免对项目造成污染

void loadLocalLanguage(const std::string &path = "i8n");

std::string HKU_API htr(const char *id);

// 按上下文获取翻译
std::string HKU_API hctr(const char *ctx, const char *id);

}  // namespace hku