/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"

namespace hku {

/**
 * @brief 激活设备
 * @param active_code 授权码
 */
void HKU_API activateDevice(const std::string& active_code);

// 查看当前授权
void HKU_API viewLicense();

}  // namespace hku
