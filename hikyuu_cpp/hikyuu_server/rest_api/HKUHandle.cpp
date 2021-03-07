/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#include <unordered_map>
#include "HKUHandle.h"

namespace hku {

enum hku_rest_errno {
    HKU_HANDLE_INVALID_CONTENT = 1,
    HKU_HANDLE_MISS_TOKEN = 2,
    HKU_HANDLE_UNAUTHORIZED = 3,
};

static std::unordered_map<int, const char *> g_hku_handle_errmsg{
  // clang-format off
  {HKU_HANDLE_INVALID_CONTENT, "Invalid Content-Type, Please use 'application/json'"},
  {HKU_HANDLE_MISS_TOKEN, "Missing token"},
  {HKU_HANDLE_UNAUTHORIZED, "Authentication failed"},
  // clang-format on
};

void HKUHandle::before_run() {
    setResHeader("Content-Type", "application/json; charset=UTF-8");

    const char *content_type = getReqHeader("Content-Type");
    HANDLE_CHECK(content_type, HKU_HANDLE_INVALID_CONTENT);

    const char *token = getReqHeader("token");
    HANDLE_CHECK(token, HKU_HANDLE_MISS_TOKEN);
}

void HKUHandle::error(int errcode) {
    auto iter = g_hku_handle_errmsg.find(errcode);
    if (iter == g_hku_handle_errmsg.end()) {
        private_error(errcode);
        return;
    }

    // http 响应状态设为 400
    setResStatus(NNG_HTTP_STATUS_BAD_REQUEST);
    setResData(fmt::format(R"({{"error": "{}", "msg": "{}"}})", errcode, iter->second));
}

}  // namespace hku