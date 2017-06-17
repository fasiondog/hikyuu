/*
 * MySQLKDataDriverImp.h
 *
 *  Created on: 2014年9月3日
 *      Author: fasiondog
 */

#ifndef MYSQLKDATADRIVERIMP_H_
#define MYSQLKDATADRIVERIMP_H_

#if defined(BOOST_WINDOWS)
    #include <mysql.h>
#else
    #include <mysql/mysql.h>
#endif

#include "../../KDataDriverImp.h"

namespace hku {

class MySQLKDataDriverImp: public KDataDriverImp {
public:
    MySQLKDataDriverImp(const shared_ptr<IniParser>&, const string& section);
    virtual ~MySQLKDataDriverImp();

    virtual void loadKData(const string& market, const string& code,
            KQuery::KType kType, size_t start_ix, size_t end_ix,
            KRecordList* out_buffer);

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
    shared_ptr<MYSQL> m_mysql;
};

} /* namespace hku */

#endif /* MYSQLKDATADRIVERIMP_H_ */
