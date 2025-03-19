/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-18
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_PLUGIN_BASE_H_
#define HKU_UTILS_PLUGIN_BASE_H_

#include "hikyuu/utilities/config.h"
#include "hikyuu/utilities/Parameter.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

class HKU_UTILS_API PluginBase {
    PARAMETER_SUPPORT

public:
    PluginBase() = default;
    virtual ~PluginBase() = default;
};

}  // namespace hku

#endif /* HKU_UTILS_PLUGIN_BASE_H_ */