/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-19
 *      Author: fasiondog
 */

#pragma once
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class TestPlugin : public PluginBase {
public:
    TestPlugin() = default;
    virtual ~TestPlugin() = default;

    virtual std::string name() const = 0;
    // virtual std::string version() const override;
    // virtual std::string description() const override;
    // virtual std::string author() const override;
    // virtual std::string license() const override;
    // virtual std::string url() const override;
    // virtual std::stringcopyright() const override;
};

}  // namespace hku

// extern "C" {
// __declspec(dllexport) hku::PluginBase* createPlugin();
// }