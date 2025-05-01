/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-08
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class DataServerPluginInterface : public PluginBase {
public:
    DataServerPluginInterface() = default;
    virtual ~DataServerPluginInterface() = default;

    virtual void start(const std::string& addr, size_t work_num) noexcept = 0;
    virtual void stop() noexcept = 0;
};

}  // namespace hku
