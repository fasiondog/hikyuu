/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-18
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_PLUGIN_BASE_H_
#define HKU_UTILS_PLUGIN_BASE_H_

#include <string>
#include "hikyuu/utilities/config.h"
#include "hikyuu/utilities/osdef.h"
#include "hikyuu/utilities/Log.h"

namespace hku {

class PluginBase {
public:
    PluginBase() = default;
    virtual ~PluginBase() = default;

    /**
     * @brief Return the plugin information
     * @details
     * The plugin information is in the json format, containing the name (string), version (int),
     * description (string) and author (string) fields
     * e.g.: {"name":"unknown","version": 1.0,"description":"","author":"unknown"}
     * @return std::string
     */
    virtual std::string info() const noexcept = 0;
};

}  // namespace hku

#if HKU_OS_WINDOWS
#define HKU_PLUGIN_DEFINE(plugin)                                      \
    extern "C" __declspec(dllexport) hku::PluginBase* createPlugin() { \
        try {                                                          \
            return new plugin();                                       \
        } catch (const std::exception& e) {                            \
            HKU_ERROR("{}", e.what());                                 \
            return nullptr;                                            \
        } catch (...) {                                                \
            return nullptr;                                            \
        }                                                              \
    }
#else
#define HKU_PLUGIN_DEFINE(plugin)                \
    extern "C" hku::PluginBase* createPlugin() { \
        try {                                    \
            return new plugin();                 \
        } catch (const std::exception& e) {      \
            HKU_ERROR("{}", e.what());           \
            return nullptr;                      \
        } catch (...) {                          \
            return nullptr;                      \
        }                                        \
    }
#endif

#endif /* HKU_UTILS_PLUGIN_BASE_H_ */