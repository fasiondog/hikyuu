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

class HKU_UTILS_API MOHelper {
public:
    static void init();

    static std::string translate(const std::string &lang, const char *id) {
        auto iter = ms_dict.find(lang);
        return iter != ms_dict.end() ? ms_dict[lang].Lookup(id) : std::string(id);
    }

    static std::string translate(const std::string &lang, const char *ctx, const char *id) {
        auto iter = ms_dict.find(lang);
        return iter != ms_dict.end() ? ms_dict[lang].LookupWithContext(ctx, id) : std::string(id);
    }

private:
    static std::unordered_map<std::string, moFileLib::moFileReader> ms_dict;
};

}  // namespace hku