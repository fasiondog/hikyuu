/*
 * TdxKDataDriver.h
 *
 *  Created on: 2017年10月12日
 *      Author: fasiondog
 */

#ifndef DATA_DRIVER_KDATA_TDX_TDXKDATADRIVER_H_
#define DATA_DRIVER_KDATA_TDX_TDXKDATADRIVER_H_

#include "../../KDataDriver.h"

namespace hku {

class TdxKDataDriver: public KDataDriver {
public:
    TdxKDataDriver();
    virtual ~TdxKDataDriver();

    virtual bool _init();

    virtual void loadKData(const string& market, const string& code,
            KQuery::KType kType, size_t start_ix, size_t end_ix,
            KRecordListPtr out_buffer) ;
    virtual size_t getCount(const string& market, const string& code,
            KQuery::KType kType);
    virtual bool getIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end);
    virtual KRecord getKRecord(const string& market, const string& code,
              size_t pos, KQuery::KType kType);

private:
    string _getFileName(const string& market, const string& code, KQuery::KType);
    size_t _getBaseCount(const string& market, const string& code, KQuery::KType);
    KRecord _getDayKRecord(const string& market, const string& code,
                           size_t pos, KQuery::KType ktype);
    KRecord _getMinKRecord(const string& market, const string& code,
                           size_t pos, KQuery::KType ktype);

    void _loadDayKData(const string& market, const string& code,
            KQuery::KType ktype, size_t start_ix, size_t end_ix,
            KRecordListPtr out_buffer);
    void _loadMinKData(const string& market, const string& code,
            KQuery::KType ktype, size_t start_ix, size_t end_ix,
            KRecordListPtr out_buffer);

    bool _getDayIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end);

    bool _getMinIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end);

private:
    string m_dirname;
};

} /* namespace hku */

#endif /* DATA_DRIVER_KDATA_TDX_TDXKDATADRIVER_H_ */
