/*
 * MySQLKDataDriverImp.h
 *
 *  Created on: 2014年9月3日
 *      Author: fasiondog
 */

#pragma once
#ifndef MYSQLKDATADRIVERIMP_H_
#define MYSQLKDATADRIVERIMP_H_

#include "../../../utilities/db_connect/DBConnect.h"
#include "../../../utilities/db_connect/mysql/MySQLConnect.h"
#include "../../KDataDriver.h"

#if defined(_MSC_VER)
#include <mysql.h>
#else
#include <mysql/mysql.h>
#endif

namespace hku {

class MySQLKDataDriver : public KDataDriver {
public:
    MySQLKDataDriver();
    virtual ~MySQLKDataDriver();

    virtual KDataDriverPtr _clone() override {
        return std::make_shared<MySQLKDataDriver>();
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

    virtual TimeLineList getTimeLineList(const string& market, const string& code,
                                         const KQuery& query) override;

    virtual TransList getTransList(const string& market, const string& code,
                                   const KQuery& query) override;

private:
    string _getTableName(const string& market, const string& code, KQuery::KType ktype);
    KRecordList _getKRecordList(const string& market, const string& code, KQuery::KType kType,
                                size_t start_ix, size_t end_ix);
    KRecordList _getKRecordList(const string& market, const string& code, KQuery::KType ktype,
                                Datetime start_date, Datetime end_date);

    TimeLineList _getTimeLineListByDate(const string& market, const string& code,
                                        const KQuery& query);
    TimeLineList _getTimeLineListByIndex(const string& market, const string& code,
                                         const KQuery& query);
    TransList _getTransListByDate(const string& market, const string& code, const KQuery& query);
    TransList _getTransListByIndex(const string& market, const string& code, const KQuery& query);

private:
    MySQLConnect* m_connect;
};

} /* namespace hku */

#endif /* MYSQLKDATADRIVERIMP_H_ */
