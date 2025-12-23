/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-06
 *      Author: fasiondog
 */

#include "interface/plugins.h"
#include "KDataToClickHouseImporter.h"

namespace hku {

KDataToClickHouseImporter::KDataToClickHouseImporter() {
    auto& sm = StockManager::instance();
    m_plugin =
      sm.getPlugin<ImportKDataToClickHousePluginInterface>(HKU_PLUGIN_IMPORTKDATATOCLICKHOUSE);
    if (!m_plugin) {
        HKU_ERROR(htr("Can't find {} plugin!", HKU_PLUGIN_IMPORTKDATATOCLICKHOUSE));
    }
}

KDataToClickHouseImporter::~KDataToClickHouseImporter() {}

bool KDataToClickHouseImporter::setConfig(const string& host, int port, const string& user,
                                          const string& pwd) {
    return m_plugin ? m_plugin->setConfig(host, port, user, pwd) : false;
}

Datetime KDataToClickHouseImporter::getLastDatetime(const string& market, const string& code,
                                                    const KQuery::KType& ktype) {
    return m_plugin ? m_plugin->getLastDatetime(market, code, ktype) : Null<Datetime>();
}

void KDataToClickHouseImporter::addKRecordList(const string& market, const string& code,
                                               const vector<KRecord>& krecords,
                                               const KQuery::KType& ktype) {
    if (m_plugin) {
        m_plugin->addKRecordList(market, code, krecords, ktype);
    }
}

void KDataToClickHouseImporter::updateIndex(const string& market, const string& code,
                                            const KQuery::KType& ktype) {
    if (m_plugin) {
        m_plugin->updateIndex(market, code, ktype);
    }
}

void KDataToClickHouseImporter::remove(const string& market, const string& code,
                                       const KQuery::KType& ktype, Datetime start) {
    if (m_plugin) {
        m_plugin->remove(market, code, ktype, start);
    }
}

void KDataToClickHouseImporter::addTimeLineList(const string& market, const string& code,
                                                const TimeLineList& timeline) {
    if (m_plugin) {
        m_plugin->addTimeLineList(market, code, timeline);
    }
}

void KDataToClickHouseImporter::addTransList(const string& market, const string& code,
                                             const TransRecordList& translist) {
    if (m_plugin) {
        m_plugin->addTransList(market, code, translist);
    }
}

}  // namespace hku