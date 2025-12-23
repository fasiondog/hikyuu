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

void loadLocalLanguage(const std::string &path) {
    std::string lang = getSystemLanguage();
    if (lang == "zh_cn") {
        auto &reader = moFileLib::moFileReaderSingleton::GetInstance();
        reader.ReadFile(fmt::format("{}/zh_CN/hikyuu.mo", path).c_str());
        reader.ReadFile(fmt::format("{}/zh_CN/hikyuu_plugin.mo", path).c_str());
    }
}

std::string HKU_API lang_htr(const char *id) {
    return moFileLib::moFileReaderSingleton::GetInstance().Lookup(id);
}

std::string HKU_API lang_hctr(const char *ctx, const char *id) {
    return moFileLib::moFileReaderSingleton::GetInstance().LookupWithContext(ctx, id);
}

}  // namespace hku