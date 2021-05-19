/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#pragma once

#include <stdlib.h>
#include "http/HttpHandle.h"
#include "db/db.h"  // 这里统一引入
#include "RestErrorCode.h"
#include "filter.h"

namespace hku {

class NoAuthRestHandle : public HttpHandle {
    CLASS_LOGGER(NoAuthRestHandle)

public:
    NoAuthRestHandle(nng_aio *aio) : HttpHandle(aio) {
        // addFilter(AuthorizeFilter);
    }

    virtual ~NoAuthRestHandle() {}

    virtual void before_run() override {
        setResHeader("Content-Type", "application/json; charset=UTF-8");
        req = getReqJson();
    }

    virtual void after_run() override {
        // 强制关闭连接，即仅有短连接
        // nng_http_res_set_status(m_nng_res, NNG_HTTP_STATUS_OK);
        std::string lang = getLanguage();
        setResData(res);
    }

protected:
    void check_missing_param(const char *param) {
        if (!req.contains(param)) {
            throw HttpBadRequestError(BadRequestErrorCode::MISS_PARAMETER,
                                      fmt::format(R"(Missing param "{}")", param));
        }
    }

    template <typename ModelTable>
    void check_enum_field(const std::string &field, const std::string &value) {
        if (!DB::isValidEumValue(ModelTable::getTableName(), field, value)) {
            throw HttpBadRequestError(BadRequestErrorCode::WRONG_PARAMETER,
                                      fmt::format("Invalid field({}) value: {}", field, value));
        }
    }

protected:
    json req;  // 子类在 run 方法中，直接使用次req
    json res;
};

class RestHandle : public NoAuthRestHandle {
    CLASS_LOGGER(RestHandle)

public:
    RestHandle(nng_aio *aio) : NoAuthRestHandle(aio) {
        addFilter(AuthorizeFilter);
    }

    virtual ~RestHandle() {}

    virtual void run() override {}

    virtual void after_run() override {
        if (!m_update_token.empty()) {
            res["update_token"] = m_update_token;
        }
        res["result"] = true;
        setResData(res);
        // 强制关闭连接，即仅有短连接
        // nng_http_res_set_status(m_nng_res, NNG_HTTP_STATUS_OK);
    }

    void setCurrentUserId(uint64_t userid) {
        m_user_id = userid;
    }

    uint64_t getCurrentUserId() const {
        return m_user_id;
    }

    void setUpdateToken(const std::string &token) {
        m_update_token = token;
    }

private:
    uint64_t m_user_id;
    std::string m_update_token;
};

#define NO_AUTH_REST_HANDLE_IMP(cls) \
public:                              \
    cls(nng_aio *aio) : NoAuthRestHandle(aio) {}

#define REST_HANDLE_IMP(cls) \
public:                      \
    cls(nng_aio *aio) : RestHandle(aio) {}

}  // namespace hku