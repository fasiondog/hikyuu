/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#pragma once

#include <string>

namespace hku {

std::string url_escape(const char* istr);
std::string url_unescape(const char* istr);

}  // namespace hku