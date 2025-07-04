/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-05-02
 *     Author: fasiondog
 */

#include "hikyuu/utilities/os.h"
#include "hikyuu/utilities/arithmetic.h"
#include "hikyuu/utilities/Log.h"
#include "mo.h"

#include "hikyuu/utilities/osdef.h"
#if HKU_OS_WINDOWS
#include <windows.h>
#elif HKU_OS_OSX
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace hku {
namespace mo {

std::unordered_map<std::string, moFileLib::moFileReader> g_mo_dict;
std::string g_mo_sys_language;
moFileLib::moFileReader *g_ptr_mo_sys_moreader{nullptr};

void HKU_UTILS_API init(const std::string &path) {
    g_mo_sys_language = getSystemLanguage();
    g_ptr_mo_sys_moreader = nullptr;
    if (g_mo_sys_language == "zh_cn") {
        std::string filename = fmt::format("{}/zh_CN.mo", path);
        HKU_WARN_IF_RETURN(
          !existFile(filename), void(),
          fmt::format("There is no internationalized language file: {}", filename));
        g_mo_dict[g_mo_sys_language] = moFileLib::moFileReader();
        g_mo_dict[g_mo_sys_language].ReadFile(filename.c_str());
        g_ptr_mo_sys_moreader = &g_mo_dict[g_mo_sys_language];
    }
}

std::string HKU_UTILS_API translate(const std::string &lang, const char *id) {
    auto iter = g_mo_dict.find(lang);
    return iter != g_mo_dict.end() ? g_mo_dict[lang].Lookup(id) : std::string(id);
}

std::string HKU_UTILS_API translate(const std::string &lang, const char *ctx, const char *id) {
    auto iter = g_mo_dict.find(lang);
    return iter != g_mo_dict.end() ? g_mo_dict[lang].LookupWithContext(ctx, id) : std::string(id);
}

#if HKU_OS_WINDOWS
std::string HKU_UTILS_API getSystemLanguage() {
    LCID lcid = GetUserDefaultUILanguage();
    char lang[256];

    std::string ret;
    if (GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, lang, sizeof(lang)) == 0) {
        return ret;
    }

    ret = std::string(lang);
    to_lower(ret);
    return ret;
}

#elif HKU_OS_LINUX
std::string HKU_UTILS_API getSystemLanguage() {
    std::string ret;
    const char *langEnv = std::getenv("LANG");
    HKU_IF_RETURN(langEnv == nullptr, ret);

    std::string lang(langEnv);
    auto ss = split(lang, '.');
    ret = std::string(ss[0]);
    to_lower(ret);
    return ret;
}

#elif HKU_OS_OSX
std::string HKU_UTILS_API getSystemLanguage() {
    CFLocaleRef currentLocale = CFLocaleCopyCurrent();

    // 显式类型转换
    CFStringRef languageCode = (CFStringRef)CFLocaleGetValue(currentLocale, kCFLocaleLanguageCode);

    if (languageCode == nullptr || !(CFStringGetTypeID() == CFGetTypeID(languageCode))) {
        CFRelease(currentLocale);
        return "";
    }

    CFIndex length = CFStringGetLength(languageCode);
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
    std::unique_ptr<char[]> buffer(new char[maxSize]);

    if (CFStringGetCString(languageCode, buffer.get(), maxSize, kCFStringEncodingUTF8)) {
        std::string result(buffer.get());
        CFRelease(currentLocale);
        to_lower(result);
        if (result == "zh") {
            result = "zh_cn";
        }
        return result;
    }

    CFRelease(currentLocale);
    return "";
}
#endif

std::string HKU_UTILS_API _tr(const char *id) {
    return g_ptr_mo_sys_moreader ? g_ptr_mo_sys_moreader->Lookup(id) : std::string(id);
}

std::string HKU_UTILS_API _ctr(const char *ctx, const char *id) {
    return g_ptr_mo_sys_moreader ? g_ptr_mo_sys_moreader->LookupWithContext(ctx, id)
                                 : std::string(id);
}

}  // namespace mo
}  // namespace hku