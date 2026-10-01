/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-10-01
 *      Author: fasiondog
 */

#include "hikyuu/utilities/http_client/HttpException.h"

namespace hku {

// Defined inside the library so that this class has a key function: the vtable and the typeinfo
// are emitted once here and referenced by every other translation unit, which keeps
// catch (const HttpTimeoutException&) working across a shared library boundary
HttpTimeoutException::~HttpTimeoutException() noexcept = default;

HttpResponseTooLargeException::~HttpResponseTooLargeException() noexcept = default;

}  // namespace hku
