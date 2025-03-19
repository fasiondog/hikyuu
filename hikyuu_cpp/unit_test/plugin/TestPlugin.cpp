/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-19
 *      Author: fasiondog
 */

#include "TestPlugin.h"

namespace hku {

class TestPluginImp : public TestPlugin {
public:
    TestPluginImp() {
        setParam<std::string>("name", "TestPlugin");
    }
    virtual ~TestPluginImp() = default;

    virtual std::string name() const override {
        return getParam<std::string>("name");
    }
    // virtual std::string version() const override;
    // virtual std::string description() const override;
    // virtual std::string author() const override;
    // virtual std::string license() const override;
    // virtual std::string url() const override;
    // virtual std::stringcopyright() const override;
};

}  // namespace hku

HKU_PLUGIN_EXPORT_ENTRY(hku::TestPluginImp)