/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-01
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/data_driver/KDataDriver.h"
#include "hikyuu/data_driver/BlockInfoDriver.h"
#include "hikyuu/data_driver/BaseInfoDriver.h"
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class DataDriverPluginInterface : public PluginBase {
public:
    DataDriverPluginInterface() = default;
    virtual ~DataDriverPluginInterface() = default;

    virtual KDataDriverPtr getKDataDriver() = 0;
    virtual BlockInfoDriverPtr getBlockInfoDriver() = 0;
    virtual BaseInfoDriverPtr getBaseInfoDriver() = 0;
};

}  // namespace hku
