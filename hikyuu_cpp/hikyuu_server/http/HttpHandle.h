/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include <string>
#include <nng/nng.h>
#include <nng/supplemental/http/http.h>

#include "../common/log.h"

namespace hku {

#define NNG_CHECK(rv)                                       \
    {                                                       \
        if (rv != 0) {                                      \
            CLS_THROW("[HTTP_ERROR] {}", nng_strerror(rv)); \
        }                                                   \
    }

#define NNG_CHECK_M(rv, msg)                                             \
    {                                                                    \
        if (rv != 0) {                                                   \
            CLS_THROW("[HTTP_ERROR] {} err: {}", msg, nng_strerror(rv)); \
        }                                                                \
    }

class HttpHandle {
    CLASS_LOGGER(HttpHandle)

public:
    HttpHandle() = delete;
    HttpHandle(nng_aio *aio);

    virtual void run() {
        logger()->warn("Not implemented HttpHandle run method!");
    }

    void getRequestData(void **data, size_t *len) {
        nng_http_req_get_data(m_nng_req, data, len);
    }

    std::string getRequestData();

    void setResponseStatus(uint16_t status) {
        NNG_CHECK(nng_http_res_set_status(m_nng_res, status));
    }

    void setResponseData(const std::string &content) {
        NNG_CHECK(nng_http_res_copy_data(m_nng_res, content.c_str(), content.size()));
    }

    void operator()();

private:
    void error(const std::string &errmsg);

protected:
    nng_aio *m_http_aio{nullptr};
    nng_http_res *m_nng_res{nullptr};
    nng_http_req *m_nng_req{nullptr};
    nng_http_conn *m_nng_conn{nullptr};
};

}  // namespace hku
