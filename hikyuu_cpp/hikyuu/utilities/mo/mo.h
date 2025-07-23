/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-01
 *     Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/config.h"
#if !HKU_ENABLE_MO
#error "Don't enable mo, please config with --mo=y"
#endif

#include <unordered_map>
#include "hikyuu/utilities/string_view.h"
#include "moFileReader.h"

#if defined(_MSC_VER)
// moFileReader.hpp 最后打开了4251告警，这里关闭
#pragma warning(disable : 4251)
#endif /* _MSC_VER */

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {
namespace mo {

/**
 * @brief 初始化多语言支持
 * @param path 翻译文件路径
 */
void HKU_UTILS_API init(const std::string &path = "i18n");

/**
 * @brief 按指定语言获取翻译
 * @param lang
 * @param id
 * @return std::string
 */
std::string HKU_UTILS_API translate(const std::string &lang, const char *id);

/**
 * @brief 按指定语言和上下文获取翻译
 * @param lang
 * @param ctx
 * @param id
 * @return std::string
 */
std::string HKU_UTILS_API translate(const std::string &lang, const char *ctx, const char *id);

/** 获取系统语言 */
std::string HKU_UTILS_API getSystemLanguage();

/** 按系统语言获取翻译 */
std::string HKU_UTILS_API _tr(const char *id);

/** 按系统语言和上下文获取翻译 */
std::string HKU_UTILS_API _ctr(const char *ctx, const char *id);

}  // namespace mo
}  // namespace hku