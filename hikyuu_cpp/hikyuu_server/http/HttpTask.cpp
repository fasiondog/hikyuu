/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-28
 *     Author: fasiondog
 */

#pragma once

#include "HttpTask.h"

namespace hku {

HttpTask::HttpTask(nng_aio* aio) : m_http_aio(aio) {
    HKU_CHECK(m_http_aio, "Error input *aio, is null!");
}

void HttpTask::operator()() {
    int rv = -1;
    void* data = nullptr;
    nng_http_res* res;
    if (rv = nng_http_res_alloc(&res) != 0) {
        nng_aio_finish(m_http_aio, rv);
        HKU_ERROR("Failed nng_http_res_alloc!");
        return;
    }

    nng_http_req* req = (nng_http_req*)nng_aio_get_input(m_http_aio, 0);
    nng_http_conn* conn = (nng_http_conn*)nng_aio_get_input(m_http_aio, 2);

    run();

    if (rv = nng_http_res_copy_data(res, "HttpTask", 8) != 0) {
        HKU_ERROR("Failed nng_http_res_copy_data");
        return;
    }

    nng_aio_set_output(m_http_aio, 0, res);
    nng_aio_finish(m_http_aio, 0);
}

}  // namespace hku