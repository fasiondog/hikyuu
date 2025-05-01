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
#include "hikyuu/utilities/os.h"
#include "hikyuu/utilities/Log.h"
#include "PluginBase.h"

#if HKU_OS_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace hku {

class PluginLoader final {
public:
    PluginLoader() : PluginLoader(".") {}
    explicit PluginLoader(const std::string& path) : m_path(path) {}

    PluginLoader(const PluginLoader&) = delete;
    PluginLoader(PluginLoader&&) = delete;

    ~PluginLoader() {
        unload();
    }

    template <typename T>
    T* instance() const noexcept {
        HKU_IF_RETURN(!m_plugin, nullptr);
        return dynamic_cast<T*>(m_plugin.get());
    }

    bool load(const std::string& pluginname) noexcept {
        std::string filename = getFileName(pluginname);
        HKU_WARN_IF_RETURN(!existFile(filename), false, "file({}) not exist!", filename);

#if HKU_OS_WINDOWS
        m_handle = LoadLibrary(HKU_PATH(filename).c_str());
#else
        m_handle = dlopen(filename.c_str(), RTLD_LAZY);
#endif
        HKU_WARN_IF_RETURN(!m_handle, false, "load plugin({}) failed!", filename);

        typedef PluginBase* (*CreateFunction)();
        CreateFunction createFunction =
          reinterpret_cast<CreateFunction>(getFunciton("createPlugin"));
        HKU_WARN_IF_RETURN(!createFunction, false, "Failed to get plugin({}) handle!", filename);

        m_plugin.reset(createFunction());
        if (!m_plugin) {
            HKU_ERROR("Failed to create plugin ({})!", filename);
            unload();
            m_plugin.reset();
            return false;
        }

        return true;
    }

private:
    void unload() noexcept {
        m_plugin.reset();
        if (m_handle) {
#if HKU_OS_WINDOWS
            FreeLibrary(m_handle);
#else
            dlclose(m_handle);
#endif
        }
    }

    void* getFunciton(const char* symbol) noexcept {
#if HKU_OS_WINDOWS
        void* func = GetProcAddress(m_handle, symbol);
#else
        void* func = dlsym(m_handle, symbol);
#endif
        return func;
    }

    std::string getFileName(const std::string& pluginname) const noexcept {
#if HKU_OS_WINDOWS
        return fmt::format("{}/{}.dll", m_path, pluginname);
#elif HKU_OS_LINUX
        return fmt::format("{}/lib{}.so", m_path, pluginname);
#elif HKU_OS_OSX
        return fmt::format("{}/lib{}.dylib", m_path, pluginname);
#endif
    }

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