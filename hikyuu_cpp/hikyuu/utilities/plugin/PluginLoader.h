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
#include "PluginBase.h"

#ifndef HKU_UTILS_API
#define HKU_UTILS_API
#endif

namespace hku {

class HKU_UTILS_API PluginLoader final {
public:
    PluginLoader() = default;
    ~PluginLoader();

    bool load(const std::string& filename) noexcept;

    template <typename T>
    T* instance() const noexcept {
        HKU_IF_RETURN(!m_plugin, nullptr);
        return dynamic_cast<T*>(m_plugin.get());
    }

private:
    void unload() noexcept;
    void* getFunciton(const char* symbol) noexcept;

private:
    mutable std::mutex m_mutex;
    void* m_handle{nullptr};
    std::unique_ptr<PluginBase> m_plugin{nullptr};
};

}  // namespace hku

#endif /* HKU_UTILS_PLUGIN_LOADER_H_ */