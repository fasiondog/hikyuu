/*
 * CsvKDataDriverImp.h
 *
 *  Created on: 2017年7月19日
 *      Author: fasiondog
 */

#ifndef DATA_DRIVER_KDATA_CSV_CSVKDATADRIVERIMP_H_
#define DATA_DRIVER_KDATA_CSV_CSVKDATADRIVERIMP_H_

#include "../../KDataDriverImp.h"

namespace hku {

class CsvKDataDriverImp: public KDataDriverImp {
public:
    CsvKDataDriverImp(const shared_ptr<IniParser>&, const string& dirname,
            bool is_filename = true);
    virtual ~CsvKDataDriverImp();

    virtual void loadKData(const string& market, const string& code,
            KQuery::KType kType, size_t start_ix, size_t end_ix,
            KRecordList* out_buffer);

    virtual size_t
    getCount(const string& market, const string& code,
             KQuery::KType kType) ;

    virtual bool
    getIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end);

    virtual KRecord
    getKRecord(const string& market, const string& code,
              size_t pos, KQuery::KType kType);

private:
    void _get_title_column(const string&);
    void _get_token(const string&);
    string _getFileName(const string& market, const string& code, KQuery::KType);

private:
    string m_dirname;
    bool m_is_filename;

    enum COLUMN {
        DATE = 0,
        OPEN = 1,
        HIGH = 2,
        LOW = 3,
        CLOSE = 4,
        VOLUME = 5,
        AMOUNT = 6,
        LAST = 7
    };

    size_t m_column[LAST];
    vector<string> m_token_buf;

};

} /* namespace hku */

#endif /* DATA_DRIVER_KDATA_CSV_CSVKDATADRIVERIMP_H_ */
