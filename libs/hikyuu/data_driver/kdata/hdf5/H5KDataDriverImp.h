/*
 * H5KDataDriverImp.h
 *
 *  Created on: 2014年9月2日
 *      Author: fasiondog
 */

#ifndef H5KDATADRIVERIMP_H_
#define H5KDATADRIVERIMP_H_

#include "H5Record.h"
#include "../../KDataDriverImp.h"

namespace hku {

class H5KDataDriverImp: public KDataDriverImp {
public:
    H5KDataDriverImp(const shared_ptr<IniParser>&, const string& filename);
    virtual ~H5KDataDriverImp();

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
    void H5ReadRecords(H5::DataSet&, hsize_t, hsize_t, void *);
    void H5ReadIndexRecords(H5::DataSet&, hsize_t, hsize_t, void *);

    bool _getH5FileAndGroup(const string& market,
                           const string& code,
                           KQuery::KType kType,
                           H5FilePtr& out_file,
                           H5::Group& out_group);

    KRecord _getBaseRecord(const string&, const string&, size_t, KQuery::KType);
    KRecord _getOtherRecord(const string&, const string&, size_t, KQuery::KType);

    void _loadBaseData(const string& market, const string& code,
            KQuery::KType kType, size_t start_ix, size_t end_ix,
            KRecordList* out_buffer);
    void _loadIndexData(const string& market, const string& code,
            KQuery::KType kType, size_t start_ix, size_t end_ix,
            KRecordList* out_buffer);

    bool _getBaseIndexRangeByDate(const string&, const string&, const KQuery&,
            size_t& out_start, size_t& out_end);
    bool _getOtherIndexRangeByDate(const string&, const string&, const KQuery&,
            size_t& out_start, size_t& out_end);

private:
    H5::CompType m_h5DataType;
    H5::CompType m_h5IndexType;
    H5FilePtr m_h5file;
};

} /* namespace hku */

#endif /* H5KDATADRIVERIMP_H_ */
