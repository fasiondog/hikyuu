/*
 * TdxKDataDriverImp.h
 *
 *  Created on: 2016年8月15日
 *      Author: Administrator
 */

#ifndef DATA_DRIVER_KDATA_TDX_TDXKDATADRIVERIMP_H_
#define DATA_DRIVER_KDATA_TDX_TDXKDATADRIVERIMP_H_

#include "../../KDataDriverImp.h"

namespace hku {

/*
 * 通达信K线数据读取实现
 */
class TdxKDataDriverImp: public KDataDriverImp {
public:
    TdxKDataDriverImp(const shared_ptr<IniParser>&, const string&);
    virtual ~TdxKDataDriverImp();

    virtual void loadKData(const string& market, const string& code,
            KQuery::KType kyype, size_t start_ix, size_t end_ix,
            KRecordList* out_buffer);

    virtual size_t
    getCount(const string& market, const string& code,
             KQuery::KType ktype);

    virtual bool
    getIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end);

    virtual KRecord
    getKRecord(const string& market, const string& code,
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
            KRecordList* out_buffer);
    void _loadMinKData(const string& market, const string& code,
            KQuery::KType ktype, size_t start_ix, size_t end_ix,
            KRecordList* out_buffer);

    bool _getDayIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end);

    bool _getMinIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end);

private:
    string m_dirname;
};

} /* namespace hku */

#endif /* DATA_DRIVER_KDATA_TDX_TDXKDATADRIVERIMP_H_ */
