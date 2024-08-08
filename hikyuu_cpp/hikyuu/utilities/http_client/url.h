/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_URL_H
#define HKU_UTILS_URL_H

#include <string>

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

std::string HKU_UTILS_API url_escape(const char* istr);
std::string HKU_UTILS_API url_unescape(const char* istr);

}  // namespace hku

#endif