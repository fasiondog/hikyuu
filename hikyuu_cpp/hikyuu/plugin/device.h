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
void HKU_API activeDevice(const std::string& active_code);

/** 查看授权信息 */
std::string HKU_API viewLicense();

/** 移除授权 */
void HKU_API removeLicense();

/** 获取 trial 授权 */
std::string HKU_API fetchTrialLicense(const std::string& email);

/** 检查授权是否有效 */
bool HKU_API isValidLicense();

}  // namespace hku
