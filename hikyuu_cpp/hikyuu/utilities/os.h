/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-18
 *     Author: fasiondog
 */

#pragma once

#include <stdlib.h>
#include <string>

namespace hku {

std::string getUserHome() {
    char const *home = getenv("HOME");
    if (home) {
        return std::string(home);
    }

    home = getenv("USERPROFILE");
    if (home) {
        return std::string(home);
    }

    char const *hdrive = getenv("HOMEDRIVE");
    char const *hpath = getenv("HOMEPATH");
    if (hdrive && hpath) {
        return std::string(hdrive) + std::string(hpath);
    }

    return "";
}

}  // namespace hku