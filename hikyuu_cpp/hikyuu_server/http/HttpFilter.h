/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-14
 *     Author: fasiondog
 */

#pragma once

#include "../http/HttpHandle.h"
#include "../http/HttpError.h"

namespace hku {

inline void MissContentFilter(HttpHandle *handle) {
    void *data = nullptr;
    size_t len = 0;
    handle->getReqData(&data, &len);
    HTTP_VALID_CHECK(data, HttpErrorCode::MISS_CONTENT, "Miss content!");
}

inline void ApiTokenAuthorizeFilter(HttpHandle *handle) {
    const char *token = handle->getReqHeader("token");
    HTTP_VALID_CHECK(token, HttpErrorCode::MISS_TOKEN, "Miss token!");
}

}  // namespace hku