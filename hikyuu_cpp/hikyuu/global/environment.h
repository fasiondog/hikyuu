/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#pragma once

#include <string>
#include "hikyuu/version.h"

#ifndef HKU_API
#define HKU_API
#endif

namespace hku {

/**
 * 获取Hikyuu当前版本号
 * @return 版本号
 */
std::string HKU_API getVersion();

/**
 * 获取详细版本号，包含构建时间
 */
std::string HKU_API getVersionWithBuild();

void sendProbeInfo();

}  // namespace hku