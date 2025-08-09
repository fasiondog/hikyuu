/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-09
 *      Author: fasiondog
 */

#include "hikyuu/utilities/os.h"
#include "hikyuu/utilities/arithmetic.h"
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/moFileReader.h"

#include "hikyuu/utilities/osdef.h"
#if HKU_OS_WINDOWS
#include <windows.h>
#elif HKU_OS_OSX
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "lang.h"

namespace hku {

#if HKU_OS_WINDOWS
static std::string getSystemLanguage() {
    LCID lcid = GetUserDefaultUILanguage();
    char lang[256];

    std::string ret;
    if (GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, lang, sizeof(lang)) == 0) {
        return ret;
    }

    ret = std::string(lang);
    to_lower(ret);
    if (ret == "zh") {
        ret = "zh_cn";
    }
    return ret;
}

#elif HKU_OS_LINUX
static std::string getSystemLanguage() {
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
static std::string getSystemLanguage() {
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

void loadLocalLanguage(const std::string &path) {
    std::string lang = getSystemLanguage();
    if (lang == "zh_cn") {
        std::string lang_file = fmt::format("{}/zh_CN/hikyuu.mo", path);
        auto &reader = moFileLib::moFileReaderSingleton::GetInstance();
        reader.ClearTable();
        reader.ReadFile(lang_file.c_str());
    }
}

std::string HKU_API htr(const char *id) {
    return moFileLib::moFileReaderSingleton::GetInstance().Lookup(id);
}

std::string HKU_API hctr(const char *ctx, const char *id) {
    return moFileLib::moFileReaderSingleton::GetInstance().LookupWithContext(ctx, id);
}

}  // namespace hku