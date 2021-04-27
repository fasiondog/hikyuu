/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-04-28
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include "http/HttpHandle.h"

namespace hku {

std::string createToken(uint64_t user_id);

void AuthorizeFilter(HttpHandle *handle);

}  // namespace hku