/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-06
 *      Author: fasiondog
 */

#pragma once

#include "hikyuu/DataType.h"
#include "interface/ImportKDataToClickHousePluginInterface.h"

namespace hku {

class HKU_API KDataToClickHouseImporter {
public:
    KDataToClickHouseImporter();
    virtual ~KDataToClickHouseImporter();

    bool setConfig(const string& host, int port, const string& user, const string& pwd,
                   const string& baseinfo_db = "hku_base");

    Datetime getLastDatetime(const string& market, const string& code, const KQuery::KType& ktype);

    void addKRecordList(const string& market, const string& code, const vector<KRecord>& krecords,
                        const KQuery::KType& ktype);

    void addTimeLineList(const string& market, const string& code, const TimeLineList& timeline);

    void addTransList(const string& market, const string& code, const TransRecordList& translist);

    void updateIndex(const string& market, const string& code, const KQuery::KType& ktype);

    void remove(const string& market, const string& code, const KQuery::KType& ktype,
                Datetime start);

    bool addMarket(const string& market, const string& name, const string& description,
                   const string& index_code, uint64_t open1 = 930, uint64_t close1 = 1130,
                   uint64_t open2 = 1300, uint64_t close2 = 1500);

    bool addStockType(uint32_t type_id, const string& description, uint32_t precision = 2,
                      double tick = 0.01, double tick_value = 0.01, double min_trade = 1,
                      double max_trade = 1000000);

private:
    ImportKDataToClickHousePluginInterface* m_plugin{nullptr};
};

}  // namespace hku