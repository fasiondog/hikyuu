/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-03-15
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/exception.h"

namespace hku {

struct HKU_UTILS_API HttpTimeoutException : hku::exception {
    HttpTimeoutException() : hku::exception("Http timeout!") {}
    explicit HttpTimeoutException(const char* msg) : hku::exception(msg) {}
    explicit HttpTimeoutException(const std::string& msg) : hku::exception(msg) {}

    // The out-of-line destructor is the key function: it pins the vtable and the typeinfo to a
    // single translation unit inside the library, so that the type thrown here matches the type
    // caught by a caller living in another binary (otherwise each TU emits its own private
    // typeinfo and catch (const HttpTimeoutException&) never hits)
    ~HttpTimeoutException() noexcept override;
};

/**
 * @brief The response exceeds the configured size limit
 *
 * It is thrown when the peer declares or sends a response header or body larger than the limit
 * set by AsioHttpClient::setMaxHeaderSize / AsioHttpClient::setMaxResponseSize
 */
struct HKU_UTILS_API HttpResponseTooLargeException : hku::exception {
    HttpResponseTooLargeException()
    : hku::exception("The HTTP response exceeds the configured size limit!") {}
    explicit HttpResponseTooLargeException(const char* msg) : hku::exception(msg) {}
    explicit HttpResponseTooLargeException(const std::string& msg) : hku::exception(msg) {}

    // The out-of-line destructor is the key function, see HttpTimeoutException
    ~HttpResponseTooLargeException() noexcept override;
};

}  // namespace hku