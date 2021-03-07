/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#include <unordered_map>
#include "HKUHandle.h"

namespace hku {

enum hku_handle_errno {
    HKU_HANDLE_INVALID_CONTENT = 1,
};

static std::unordered_map<int, const char *> g_hku_handle_errmsg{
  // clang-format off
  {HKU_HANDLE_INVALID_CONTENT, R"(Invalid Content-Type, Please use "application/json")"},
  // clang-format on
};

void HKUHandle::before_run() {
    const char *content_type = getReqHeader("Content-Type");
    HANDLE_CHECK(content_type, HKU_HANDLE_INVALID_CONTENT);
    // application/json
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