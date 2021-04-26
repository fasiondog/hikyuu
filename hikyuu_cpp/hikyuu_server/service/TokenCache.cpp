/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-27
 *     Author: fasiondog
 */

#include "TokenCache.h"

namespace hku {

std::mutex TokenCache::ms_mutex;
std::unordered_map<std::string, Datetime> TokenCache::ms_dict;

}  // namespace hku