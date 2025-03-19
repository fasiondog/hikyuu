/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-19
 *      Author: fasiondog
 */

#include "../test_config.h"
#include "hikyuu/utilities/plugin/PluginLoader.h"
#include "../../plugin/TestPlugin.h"

using namespace hku;

namespace hku {

class TestPluginInterface {
public:
    TestPluginInterface() {
        m_loader = std::make_unique<PluginLoader>();
        m_loader->load("/Users/fasiondog/workspace/hikyuu/hikyuu/cpp/libtestplugin.dylib");
        m_impl = m_loader->instance<TestPlugin>();
    }
    virtual ~TestPluginInterface() = default;

    string name() {
        HKU_CHECK(m_impl, "Plugin not loaded!");
        return m_impl->name();
    }

private:
    std::unique_ptr<PluginLoader> m_loader;
    TestPlugin *m_impl;
};

}  // namespace hku

TEST_CASE("test_plugin") {
    TestPluginInterface plugin;
    HKU_INFO("{}", plugin.name());
}
