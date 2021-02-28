/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "HttpHandle.h"

namespace hku {

HttpHandle::HttpHandle(nng_aio* aio) : m_http_aio(aio) {
    HKU_CHECK(m_http_aio, "Error input *aio, is null!");
}

void HttpHandle::operator()() {
    int rv = -1;
    void* data = nullptr;
    try {
        HTTP_NNG_CHECK(nng_http_res_alloc(&m_nng_res), "Failed nng_http_res_alloc!");
        m_nng_req = (nng_http_req*)nng_aio_get_input(m_http_aio, 0);
        m_nng_conn = (nng_http_conn*)nng_aio_get_input(m_http_aio, 2);

        run();

        nng_aio_set_output(m_http_aio, 0, m_nng_res);
        nng_aio_finish(m_http_aio, 0);

    } catch (std::exception& e) {
        std::string errmsg(e.what());
        HKU_ERROR(errmsg);
        error(errmsg);

    } catch (...) {
        std::string errmsg("Unknown error!");
        HKU_ERROR(errmsg);
        error(errmsg);
    }
}

void HttpHandle::error(const std::string& errmsg) {
    nng_http_res_set_status(m_nng_res, NNG_HTTP_STATUS_INTERNAL_SERVER_ERROR);
    nng_http_res_set_reason(m_nng_res, errmsg.c_str());
    nng_http_res_copy_data(m_nng_res, errmsg.c_str(), errmsg.size());
    nng_aio_set_output(m_http_aio, 0, m_nng_res);
    nng_aio_finish(m_http_aio, 0);
}

}  // namespace hku