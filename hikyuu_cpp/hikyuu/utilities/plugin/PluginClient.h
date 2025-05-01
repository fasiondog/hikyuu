/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-06
 *      Author: fasiondog
 */

#pragma once

#include "PluginLoader.h"

namespace hku {

template <typename InterfaceT>
class PluginClient : public InterfaceT {
public:
    PluginClient() = delete;
    PluginClient(const std::string &path, const std::string &filename) {
        m_loader = std::make_unique<PluginLoader>(path);
        HKU_CHECK(m_loader->load(filename), "load plugin failed! {}/{}", path, filename);
        m_impl = m_loader->instance<InterfaceT>();
    }
    virtual ~PluginClient() = default;

    PluginClient(const PluginClient &) = delete;
    PluginClient &operator=(const PluginClient &) = delete;

    PluginClient(PluginClient &&rhs) : m_impl(rhs.m_impl), m_loader(std::move(rhs.m_loader)) {
        rhs.m_impl = nullptr;
    }

    PluginClient &operator=(PluginClient &&rhs) {
        if (this != &rhs) {
            m_loader = std::move(rhs.m_loader);
            m_impl = rhs.m_impl;
            rhs.m_impl = nullptr;
        }
    }

    std::string info() const noexcept override {
        return m_impl->info();
    }

    InterfaceT *getPlugin() const {
        return m_impl;
    }

protected:
    InterfaceT *m_impl{nullptr};

protected:
    std::unique_ptr<PluginLoader> m_loader;
};

}  // namespace hku
