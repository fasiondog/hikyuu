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
#include <shared_mutex>
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
        HKU_TRACE_IF_RETURN(!m_plugins.empty(), void(),
                            "Existing loaded plugins, Ignore set plugin path: {}, ", plugin_path);
        m_plugin_path = plugin_path;
    }

    void clear() noexcept {
        std::unique_lock<std::shared_mutex> write_lock(m_mutex);
        m_plugins.clear();
    }

    template <typename PluginInterfaceT>
    PluginInterfaceT* getPlugin(const std::string& pluginname) noexcept {
        PluginInterfaceT* ret{nullptr};
        try {
            {
                std::shared_lock<std::shared_mutex> read_lock(m_mutex);
                auto it = m_plugins.find(pluginname);
                if (it != m_plugins.end()) {
                    ret = it->second->instance<PluginInterfaceT>();
                    return ret;
                }
            }

            std::unique_ptr<PluginLoader> loader = std::make_unique<PluginLoader>(m_plugin_path);
            if (!loader->load(pluginname)) {
                HKU_DEBUG("Load plugin {} failed: {}", pluginname, loader->getFileName(pluginname));
                return ret;
            }

            {
                std::unique_lock<std::shared_mutex> write_lock(m_mutex);
                auto it = m_plugins.find(pluginname);
                if (it != m_plugins.end()) {
                    // 复用已插入的插件实例
                    ret = it->second->instance<PluginInterfaceT>();
                } else {
                    // 插入新加载的插件
                    auto [it, success] =
                      m_plugins.insert(std::make_pair(pluginname, std::move(loader)));
                    if (success) {
                        ret = it->second->instance<PluginInterfaceT>();
                    }
                }
                return ret;
            }
        } catch (const std::exception& e) {
            HKU_ERROR("Load plugin {} failed: {}", pluginname, e.what());
            ret = nullptr;
        } catch (...) {
            HKU_ERROR("Load plugin {} failed: unknown exception", pluginname);
            ret = nullptr;
        }
        return ret;
    }

private:
    std::string m_plugin_path;
    std::unordered_map<std::string, std::unique_ptr<PluginLoader>> m_plugins;
    std::shared_mutex m_mutex;
};

}  // namespace hku