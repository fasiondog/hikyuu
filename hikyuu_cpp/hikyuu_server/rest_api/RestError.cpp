/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-14
 *     Author: fasiondog
 */

#include "RestError.h"

namespace hku {

std::unordered_map<long, const char *> RestError::ms_err_msg{
  // clang-format off
  {INVALID_CONTENT, "Invalid Content-Type, Please use 'application/json'"},
  {MISS_TOKEN, "Missing token"},
  {UNAUTHORIZED, "Authentication failed"},
  // clang-format on
};

}