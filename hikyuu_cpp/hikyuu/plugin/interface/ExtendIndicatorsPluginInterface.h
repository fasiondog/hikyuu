/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-19
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/indicator/Indicator.h"
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class ExtendIndicatorsPluginInterface : public PluginBase {
public:
    ExtendIndicatorsPluginInterface() = default;
    virtual ~ExtendIndicatorsPluginInterface() = default;

    virtual Indicator getIndicator(const std::string& name, const Parameter& params) const = 0;

    virtual Indicator getIndicator(const std::string& name, const Indicator& ref_ind,
                                   const Parameter& params) const = 0;
};

}  // namespace hku
