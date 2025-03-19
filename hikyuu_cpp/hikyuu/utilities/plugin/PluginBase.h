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
#include "hikyuu/utilities/osdef.h"
#include "hikyuu/utilities/Parameter.h"

namespace hku {

class PluginBase {
    PARAMETER_SUPPORT

public:
    PluginBase() = default;
    virtual ~PluginBase() = default;
};

}  // namespace hku

#if HKU_OS_WINDOWS
#define HKU_PLUGIN_EXPORT_ENTRY(plugin)                                \
    extern "C" __declspec(dllexport) hku::PluginBase* createPlugin() { \
        return new plugin();                                           \
    }
#else
#define HKU_PLUGIN_EXPORT_ENTRY(plugin)          \
    extern "C" hku::PluginBase* createPlugin() { \
        return new plugin();                     \
    }
#endif

#endif /* HKU_UTILS_PLUGIN_BASE_H_ */