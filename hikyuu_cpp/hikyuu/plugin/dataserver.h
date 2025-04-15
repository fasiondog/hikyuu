/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-15
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"

namespace hku {

void HKU_API startDataServer(const std::string& addr = "tcp://0.0.0.0:9201", size_t work_num = 2);

void HKU_API stopDataServer();

}  // namespace hku