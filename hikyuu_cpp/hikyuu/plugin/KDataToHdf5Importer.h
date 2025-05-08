/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"
#include "interface/ImportKDataToHdf5PluginInterface.h"

namespace hku {

class HKU_API KDataToHdf5Importer {
public:
    KDataToHdf5Importer();
    virtual ~KDataToHdf5Importer();

    bool setConfig(const string& datapath, const vector<string>& markets = {"SH", "SZ", "BJ"});

    Datetime getLastDatetime(const string& market, const string& code, const KQuery::KType& ktype);

    void addKRecordList(const string& market, const string& code, const vector<KRecord>& krecords,
                        const KQuery::KType& ktype);

    void updateIndex(const string& market, const string& code, const KQuery::KType& ktype);

private:
    ImportKDataToHdf5PluginInterface* m_plugin{nullptr};
};

}  // namespace hku