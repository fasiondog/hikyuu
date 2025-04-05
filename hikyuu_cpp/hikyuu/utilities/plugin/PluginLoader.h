/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-18
 *      Author: fasiondog
 */

#pragma once
#ifndef HKU_UTILS_PLUGIN_LOADER_H_
#define HKU_UTILS_PLUGIN_LOADER_H_

#include <mutex>
#include <string>
#include <memory>
#include "hikyuu/utilities/config.h"
#include "hikyuu/utilities/osdef.h"
#include "hikyuu/utilities/Log.h"
#include "PluginBase.h"

#if HKU_OS_WINDOWS
#include <windows.h>
#endif

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

class HKU_UTILS_API PluginLoader final {
public:
    PluginLoader();
    explicit PluginLoader(const std::string& path);
    PluginLoader(const PluginLoader&) = delete;
    PluginLoader(PluginLoader&&) = delete;

    ~PluginLoader();

    bool load(const std::string& pluginname) noexcept;

    template <typename T>
    T* instance() const noexcept {
        HKU_IF_RETURN(!m_plugin, nullptr);
        return dynamic_cast<T*>(m_plugin.get());
    }

private:
    void unload() noexcept;
    void* getFunciton(const char* symbol) noexcept;
    std::string getFileName(const std::string& pluginname) const noexcept;

private:
#if HKU_OS_WINDOWS
    HMODULE m_handle{nullptr};
#else
    void* m_handle{nullptr};
#endif
    std::string m_path;
    std::unique_ptr<PluginBase> m_plugin{nullptr};
};

}  // namespace hku

#endif /* HKU_UTILS_PLUGIN_LOADER_H_ */