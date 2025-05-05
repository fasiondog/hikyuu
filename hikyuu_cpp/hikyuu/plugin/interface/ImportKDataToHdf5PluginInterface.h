/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-08
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/KRecord.h"
#include "hikyuu/KQuery.h"
#include "hikyuu/utilities/plugin/PluginBase.h"

namespace hku {

class ImportKDataToHdf5PluginInterface : public PluginBase {
public:
    ImportKDataToHdf5PluginInterface() = default;
    virtual ~ImportKDataToHdf5PluginInterface() = default;

    virtual bool setConfig(const string& datapath, const vector<string>& markets) = 0;

    virtual Datetime getLastDatetime(const string& market, const string& code,
                                     const KQuery::KType& ktype) = 0;

    virtual void addKRecordList(const string& market, const string& code,
                                const vector<KRecord>& krecords, const KQuery::KType& ktype) = 0;

    virtual void updateIndex(const string& market, const string& code,
                             const KQuery::KType& ktype) = 0;
};

}  // namespace hku
