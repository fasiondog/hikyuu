/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-14
 *     Author: fasiondog
 */

#pragma once

#include "../http/HttpHandle.h"
#include "RestFilterError.h"

namespace hku {

void TokenAuthorizeFilter(HttpHandle *handle);

}  // namespace hku