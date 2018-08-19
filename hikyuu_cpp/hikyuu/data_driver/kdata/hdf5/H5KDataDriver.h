/*
 * H5KDataDriver.h
 *
 *  Created on: 2017年10月11日
 *      Author: fasiondog
 */

#ifndef DATA_DRIVER_KDATA_HDF5_H5KDATADRIVER_H_
#define DATA_DRIVER_KDATA_HDF5_H5KDATADRIVER_H_

#include "../../KDataDriver.h"
#include "H5Record.h"

namespace hku {

class H5KDataDriver: public KDataDriver {
public:
    H5KDataDriver();
    virtual ~H5KDataDriver();

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
            KRecordListPtr out_buffer);
    void _loadIndexData(const string& market, const string& code,
            KQuery::KType kType, size_t start_ix, size_t end_ix,
            KRecordListPtr out_buffer);

    bool _getBaseIndexRangeByDate(const string&, const string&, const KQuery&,
            size_t& out_start, size_t& out_end);
    bool _getOtherIndexRangeByDate(const string&, const string&, const KQuery&,
            size_t& out_start, size_t& out_end);

private:
    H5::CompType m_h5DataType;
    H5::CompType m_h5IndexType;
    H5FilePtr m_h5file;
    map<string, H5FilePtr> m_h5file_map; // key: market+code
};

} /* namespace hku */

#endif /* DATA_DRIVER_KDATA_HDF5_H5KDATADRIVER_H_ */
