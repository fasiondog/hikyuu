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

class ImportKDataToMySQLPluginInterface : public PluginBase {
public:
    ImportKDataToMySQLPluginInterface() = default;
    virtual ~ImportKDataToMySQLPluginInterface() = default;

    virtual bool setConfig(const string& host, int port, const string& user, const string& pwd) = 0;

    virtual Datetime getLastDatetime(const string& market, const string& code,
                                     const KQuery::KType& ktype) = 0;

    virtual void addKRecordList(const string& market, const string& code,
                                const vector<KRecord>& krecords, const KQuery::KType& ktype) = 0;

    virtual void updateIndex(const string& market, const string& code,
                             const KQuery::KType& ktype) = 0;

    virtual void addTimeLineList(const string& market, const string& code,
                                 const TimeLineList& timeline) = 0;

    virtual void addTransList(const string& market, const string& code,
                              const TransRecordList& translist) = 0;

    virtual void remove(const string& market, const string& code, const KQuery::KType& ktype,
                        Datetime start) = 0;
};

}  // namespace hku
