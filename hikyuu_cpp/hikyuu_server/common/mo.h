/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-01
 *     Author: fasiondog
 */

#pragma once

#include "moFileReader.hpp"

// 多国语言支持
#define _(S) hku::g_moFR.Lookup(S)
#define _L(S) hku::moFR.Lookup(S)
#define _LC(ctx, str) hku::moFR.LookupWithContext(ctx, str)

namespace hku {

extern moFileLib::moFileReader g_moFR;

// 初始化读取mo文件
void mo_init(const char *filename);

}  // namespace hku