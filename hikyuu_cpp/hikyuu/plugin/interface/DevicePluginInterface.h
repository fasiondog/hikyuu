/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-08
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class DevicePluginInterface : public PluginBase {
public:
    DevicePluginInterface() = default;
    virtual ~DevicePluginInterface() = default;

    virtual void activate(const std::string& active_code) noexcept = 0;
    virtual void viewLicense() noexcept = 0;
    virtual void removeLicense() noexcept = 0;
    virtual void fetchTrialLicense(const std::string& email) noexcept = 0;
};

}  // namespace hku
