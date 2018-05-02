/*
 * MySQLKDataDriverImp.h
 *
 *  Created on: 2014年9月3日
 *      Author: fasiondog
 */

#ifndef MYSQLKDATADRIVERIMP_H_
#define MYSQLKDATADRIVERIMP_H_

#include "../../KDataDriver.h"

#if defined(BOOST_WINDOWS)
    #include <mysql.h>
#else
    #include <mysql/mysql.h>
#endif

namespace hku {

class MySQLKDataDriver: public KDataDriver {
public:
    MySQLKDataDriver();
    virtual ~MySQLKDataDriver();

    virtual bool _init();

    virtual void loadKData(const string& market, const string& code,
            KQuery::KType kType, size_t start_ix, size_t end_ix,
            KRecordListPtr out_buffer);

    virtual size_t getCount(const string& market,
                            const string& code,
                            KQuery::KType kType);

    virtual bool
    getIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end);

    virtual KRecord
    getKRecord(const string& market, const string& code,
              size_t pos, KQuery::KType kType);

private:
    string _getTableName(const string& market,
                        const string& code,
                        KQuery::KType ktype);
    bool _query(const string& sql_str);

private:
    shared_ptr<MYSQL> m_mysql;
    string m_host;
    unsigned int m_port;
    string m_usr;
    string m_pwd;
};

} /* namespace hku */

#endif /* MYSQLKDATADRIVERIMP_H_ */
