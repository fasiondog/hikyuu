/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#pragma once

#include "http/HttpHandle.h"
#include "db/db.h"  // 这里统一引入
#include "RestErrorCode.h"

namespace hku {

inline void AuthorizeFilter(HttpHandle *handle) {
    const char *token = handle->getReqHeader("hku_token");
    HTTP_CHECK(token, HttpErrorCode::MISS_TOKEN, "Miss token!");
    HTTP_CHECK(
      strcmp(token, "7c98806c0711cf996d602890e0ab9119d9a86afe04296ba69a16f0d9d76be755") == 0,
      HttpErrorCode::UNAUTHORIZED, "Failed authorize!");
}

class NoAuthRestHandle : public HttpHandle {
    CLASS_LOGGER(NoAuthRestHandle)

public:
    NoAuthRestHandle(nng_aio *aio) : HttpHandle(aio) {
        // addFilter(AuthorizeFilter);
    }

    virtual void before_run() override {
        setResHeader("Content-Type", "application/json; charset=UTF-8");
        req = getReqJson();
    }

    /*virtual void after_run() override {
        // 强制关闭连接，即仅有短连接
        nng_http_res_set_status(m_nng_res, NNG_HTTP_STATUS_OK);
    }*/

protected:
    void check_missing_param(const char *param) {
        if (!req.contains(param)) {
            throw HttpError(HttpErrorCode::MISS_PARAMETER,
                            fmt::format(R"(Missing param "{}")", param));
        }
    }

    template <typename ModelTable>
    void check_enum_field(const std::string &field, const std::string &value) {
        if (!DB::isValidEumValue(ModelTable::getTableName(), field, value)) {
            throw HttpError(RestErrorCode::REST_INVALID_VALUE,
                            fmt::format("Invalid field({}) value: {}", field, value));
        }
    }

protected:
    json req;  // 子类在 run 方法中，直接使用次req
};

class RestHandle : public NoAuthRestHandle {
    CLASS_LOGGER(RestHandle)

public:
    RestHandle(nng_aio *aio) : NoAuthRestHandle(aio) {
        addFilter(AuthorizeFilter);
    }
};

#define NO_AUTH_REST_HANDLE_IMP(cls) \
public:                              \
    cls(nng_aio *aio) : NoAuthRestHandle(aio) {}

#define REST_HANDLE_IMP(cls) \
public:                      \
    cls(nng_aio *aio) : RestHandle(aio) {}

}  // namespace hku