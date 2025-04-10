/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-10
 *      Author: fasiondog
 */

#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include "hikyuu/utilities/Log.h"
#include "hikyuu/utilities/plugin/PluginLoader.h"

namespace hku {

class PluginManager final {
public:
    PluginManager() : m_plugin_path(".") {};
    explicit PluginManager(const std::string& plugin_path) : m_plugin_path(plugin_path) {}

    ~PluginManager() = default;
    PluginManager(const PluginManager&) = delete;
    PluginManager(PluginManager&&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    PluginManager& operator=(PluginManager&&) = delete;

    const std::string& pluginPath() const noexcept {
        return m_plugin_path;
    }

    void pluginPath(const std::string& plugin_path) noexcept {
        HKU_WARN_IF_RETURN(!m_plugins.empty(), void(),
                           "Existing loaded plugins, Ignore set plugin path: {}, ", plugin_path);
        m_plugin_path = plugin_path;
    }

    template <typename PluginInterfaceT>
    PluginInterfaceT* getPlugin(const std::string& pluginname) noexcept;

private:
    std::unordered_map<std::string, std::unique_ptr<PluginLoader>>::iterator load(
      const std::string& pluginname) noexcept {
        std::unique_ptr<PluginLoader> loader = std::make_unique<PluginLoader>(m_plugin_path);
        if (loader->load(pluginname)) {
            auto [it, success] = m_plugins.insert(std::make_pair(pluginname, std::move(loader)));
            if (success) {
                return it;
            }
        }
        return m_plugins.end();
    }

private:
    std::string m_plugin_path;
    std::unordered_map<std::string, std::unique_ptr<PluginLoader>> m_plugins;
};

template <typename PluginInterfaceT>
PluginInterfaceT* PluginManager::getPlugin(const std::string& pluginname) noexcept {
    auto it = m_plugins.find(pluginname);
    if (it == m_plugins.end()) {
        it = load(pluginname);
    }
    return it != m_plugins.end() ? it->second->instance<PluginInterfaceT>() : nullptr;
}

}  // namespace hku