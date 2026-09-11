/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-06
 *      Author: fasiondog
 */

#include "interface/plugins.h"
#include "KDataToHdf5Importer.h"

namespace hku {

KDataToHdf5Importer::KDataToHdf5Importer() {
    auto& sm = StockManager::instance();
    m_plugin = sm.getPlugin<ImportKDataToHdf5PluginInterface>(HKU_PLUGIN_IMPORTKDATATOHDF5);
    if (!m_plugin) {
        HKU_ERROR(htr("Can't find {} plugin!", HKU_PLUGIN_IMPORTKDATATOHDF5));
    }
}

KDataToHdf5Importer::~KDataToHdf5Importer() {}

bool KDataToHdf5Importer::setConfig(const string& datapath, const vector<string>& markets,
                                    const vector<string>& ktypes, const string& baseinfo_path) {
    return m_plugin ? m_plugin->setConfig(datapath, markets, ktypes, baseinfo_path) : false;
}

Datetime KDataToHdf5Importer::getLastDatetime(const string& market, const string& code,
                                              const KQuery::KType& ktype) {
    return m_plugin ? m_plugin->getLastDatetime(market, code, ktype) : Null<Datetime>();
}

void KDataToHdf5Importer::addKRecordList(const string& market, const string& code,
                                         const vector<KRecord>& krecords,
                                         const KQuery::KType& ktype) {
    if (m_plugin) {
        m_plugin->addKRecordList(market, code, krecords, ktype);
    }
}

void KDataToHdf5Importer::updateIndex(const string& market, const string& code,
                                      const KQuery::KType& ktype) {
    if (m_plugin) {
        m_plugin->updateIndex(market, code, ktype);
    }
}

void KDataToHdf5Importer::remove(const string& market, const string& code,
                                 const KQuery::KType& ktype, Datetime start) {
    if (m_plugin) {
        m_plugin->remove(market, code, ktype, start);
    }
}

void KDataToHdf5Importer::addTimeLineList(const string& market, const string& code,
                                          const TimeLineList& timeline) {
    if (m_plugin) {
        m_plugin->addTimeLineList(market, code, timeline);
    }
}

void KDataToHdf5Importer::addTransList(const string& market, const string& code,
                                       const TransRecordList& translist) {
    if (m_plugin) {
        m_plugin->addTransList(market, code, translist);
    }
}

bool KDataToHdf5Importer::addMarket(const string& market, const string& name,
                                    const string& description, const string& index_code,
                                    uint64_t open1, uint64_t close1, uint64_t open2,
                                    uint64_t close2) {
    return m_plugin ? m_plugin->addMarket(market, name, description, index_code, open1, close1,
                                          open2, close2)
                    : false;
}

bool KDataToHdf5Importer::addStockType(uint32_t type_id, const string& description,
                                       uint32_t precision, double tick, double tick_value,
                                       double min_trade, double max_trade) {
    return m_plugin ? m_plugin->addStockType(type_id, description, precision, tick, tick_value,
                                             min_trade, max_trade)
                    : false;
}

}  // namespace hku