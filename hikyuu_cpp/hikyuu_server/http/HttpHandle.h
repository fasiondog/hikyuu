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

#define HTTP_NNG_CHECK(rv, msg)                                          \
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

    nng_aio *get_http_aio() const {
        return m_http_aio;
    }

    nng_http_res *get_nng_res() const {
        return m_nng_res;
    }

    nng_http_req *get_nng_req() const {
        return m_nng_req;
    }

    nng_http_conn *get_nng_conn() const {
        return m_nng_conn;
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
