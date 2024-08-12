/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-08-13
 *      Author: fasiondog
 */

#include "exception.h"

namespace hku {

const char *exception::what() const noexcept {
    return m_msg.c_str();
}

}  // namespace hku