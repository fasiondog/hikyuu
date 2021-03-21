/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-03-07
 *     Author: fasiondog
 */

#include "../http/HttpFilter.h"
#include "RestHandle.h"

namespace hku {

RestHandle::RestHandle(nng_aio *aio) : HttpHandle(aio) {
    addFilter(ApiTokenAuthorizeFilter);
}

void RestHandle::before_run() {
    setResHeader("Content-Type", "application/json; charset=UTF-8");
}

}  // namespace hku