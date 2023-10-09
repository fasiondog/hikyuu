/*
 * SQLiteKDataDriver.h
 * 
 *   Created on: 2023年09月14日
 *       Author: yangrq1018
 */
#pragma once
#ifndef SQLITE_KDATA_DRIVER_H
#define SQLITE_KDATA_DRIVER_H

#include "../../../utilities/db_connect/DBConnect.h"
#include "../../../utilities/db_connect/sqlite/SQLiteConnect.h"
#include "../../KDataDriver.h"

namespace hku {

class SQLiteKDataDriver : public KDataDriver {
public:
    SQLiteKDataDriver();
    virtual ~SQLiteKDataDriver();

    virtual KDataDriverPtr _clone() override {
        return std::make_shared<SQLiteKDataDriver>();
    }

    virtual bool _init() override;

    virtual bool isIndexFirst() override {
        return false;
    }

    virtual bool canParallelLoad() override {
        return true;
    }

    virtual size_t getCount(const string& market, const string& code, KQuery::KType kType) override;

    virtual bool getIndexRangeByDate(const string& market, const string& code, const KQuery& query,
                                     size_t& out_start, size_t& out_end) override;

    virtual KRecordList getKRecordList(const string& market, const string& code,
                                       const KQuery& query) override;

private:
    string _getTableName(const string& market, const string& code, KQuery::KType ktype);
    KRecordList _getKRecordList(const string& market, const string& code, KQuery::KType kType,
                                size_t start_ix, size_t end_ix);
    KRecordList _getKRecordList(const string& market, const string& code, KQuery::KType ktype,
                                Datetime start_date, Datetime end_date);
    static KRecordList convertToNewInterval(const KRecordList& candles, KQuery::KType from_type,
                                 KQuery::KType to_ktype);

private:
    unordered_map<string, SQLiteConnectPtr> m_sqlite_connection_map;  // key: exchange+code
    bool m_ifConvert = false;
};

} /* namespace hku */

#endif /* SQLITE_KDATA_DRIVER_H */
