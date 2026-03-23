/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-03-15
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/exception.h"

namespace hku {

struct HttpTimeoutException : hku::exception {
    HttpTimeoutException() : hku::exception("Http timeout!") {}
    explicit HttpTimeoutException(const char* msg) : hku::exception(msg) {}
    explicit HttpTimeoutException(const std::string& msg) : hku::exception(msg) {}
    virtual ~HttpTimeoutException() noexcept override = default;
};

}  // namespace hku