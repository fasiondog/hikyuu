/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-18
 *      Author: fasiondog
 */

#include "hikyuu/utilities/osdef.h"
#include "hikyuu/utilities/os.h"
#include "PluginLoader.h"

#if HKU_OS_WINDOWS
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace hku {

PluginLoader::PluginLoader() : PluginLoader(".") {}

PluginLoader::PluginLoader(const std::string& path) : m_path(path) {}

PluginLoader::~PluginLoader() {
    unload();
}

std::string PluginLoader::getFileName(const std::string& pluginname) const noexcept {
#if HKU_OS_WINDOWS
    return fmt::format("{}/{}.dll", m_path, pluginname);
#elif HKU_OS_LINUX
    return fmt::format("{}/lib{}.so", m_path, pluginname);
#elif HKU_OS_OSX
    return fmt::format("{}/lib{}.dylib", m_path, pluginname);
#endif
}

void* PluginLoader::getFunciton(const char* symbol) noexcept {
#if HKU_OS_WINDOWS
    void* func = GetProcAddress(m_handle, symbol);
#else
    void* func = dlsym(m_handle, symbol);
#endif
    return func;
}

bool PluginLoader::load(const std::string& pluginname) noexcept {
    std::string filename = getFileName(pluginname);
    HKU_WARN_IF_RETURN(!existFile(filename), false, "file({}) not exist!", filename);

#if HKU_OS_WINDOWS
    m_handle = LoadLibrary(HKU_PATH(filename).c_str());
#else
    m_handle = dlopen(filename.c_str(), RTLD_LAZY);
#endif
    HKU_WARN_IF_RETURN(!m_handle, false, "load plugin({}) failed!", filename);

    typedef PluginBase* (*CreateFunction)();
    CreateFunction createFunction = reinterpret_cast<CreateFunction>(getFunciton("createPlugin"));
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

void PluginLoader::unload() noexcept {
    m_plugin.reset();
    if (m_handle) {
#if HKU_OS_WINDOWS
        FreeLibrary(m_handle);
#else
        dlclose(m_handle);
#endif
    }
}

}  // namespace hku