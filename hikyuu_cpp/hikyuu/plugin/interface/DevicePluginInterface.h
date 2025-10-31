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

    virtual void bind(const std::string& email, const std::string& active_code) noexcept = 0;
    virtual void activate(const std::string& active_code, bool replace) noexcept = 0;
    virtual std::string viewLicense() noexcept = 0;
    virtual void removeLicense() noexcept = 0;
    virtual std::string fetchTrialLicense(const std::string& email) noexcept = 0;
    virtual bool isValidLicsense() noexcept = 0;
    virtual Datetime getExpireDate() const noexcept = 0;
};

}  // namespace hku
