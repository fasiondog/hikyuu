/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-14
 *     Author: fasiondog
 */

#include "RestFilterError.h"

namespace hku {

std::unordered_map<RestFilterError::Code, const char *> RestFilterError::ms_err_msg{
  // clang-format off
  {RestFilterError::Code::MISS_TOKEN, "Missing token"},
  {RestFilterError::Code::UNAUTHORIZED, "Authentication failed"},
  {RestFilterError::Code::INVALID_CONTENT_TYPE, "Invalid Content-Type, Please use 'application/json'"},
  // clang-format on
};

}  // namespace hku