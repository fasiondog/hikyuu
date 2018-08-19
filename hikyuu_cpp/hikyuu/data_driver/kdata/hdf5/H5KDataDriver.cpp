/*
 * H5KDataDriver.cpp
 *
 *  Created on: 2017年10月11日
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "H5KDataDriver.h"

namespace hku {

class Hdf5FileCloser{
public:
    void operator()(H5::H5File *h5file){
        if(h5file){
            //string filename(h5file->getFileName());
            h5file->close();
            //HKU_TRACE("Closed " << filename);
        }
    }
};

H5KDataDriver::H5KDataDriver(): KDataDriver("hdf5") {
    m_h5DataType = H5::CompType(sizeof(H5Record));
    m_h5DataType.insertMember("datetime",HOFFSET(H5Record,datetime),H5::PredType::NATIVE_UINT64);
    m_h5DataType.insertMember("openPrice",HOFFSET(H5Record,openPrice),H5::PredType::NATIVE_UINT);
    m_h5DataType.insertMember("highPrice",HOFFSET(H5Record,highPrice),H5::PredType::NATIVE_UINT);
    m_h5DataType.insertMember("lowPrice",HOFFSET(H5Record,lowPrice),H5::PredType::NATIVE_UINT);
    m_h5DataType.insertMember("closePrice",HOFFSET(H5Record,closePrice),H5::PredType::NATIVE_UINT);
    m_h5DataType.insertMember("transAmount",HOFFSET(H5Record,transAmount),H5::PredType::NATIVE_UINT64);
    m_h5DataType.insertMember("transCount",HOFFSET(H5Record,transCount),H5::PredType::NATIVE_UINT64);

    m_h5IndexType = H5::CompType(sizeof(H5IndexRecord));
    m_h5IndexType.insertMember("datetime",HOFFSET(H5IndexRecord,datetime),H5::PredType::NATIVE_UINT64);
    m_h5IndexType.insertMember("start",HOFFSET(H5IndexRecord,start),H5::PredType::NATIVE_UINT64);
}

H5KDataDriver::~H5KDataDriver() {

}

bool H5KDataDriver::_init() {
    //关闭HDF异常自动打印
    H5::Exception::dontPrint();

    StringList keys = m_params.getNameList();
    string filename;
    for (auto iter = keys.begin(); iter != keys.end(); ++iter) {
        size_t pos = iter->find("_");
        if (pos == string::npos || pos == 0 || pos == iter->size()-1)
            continue;

        string market = iter->substr(0, pos);
        string ktype = iter->substr(pos+1);
        boost::to_upper(market);
        boost::to_upper(ktype);

        try {
            if (ktype == KQuery::getKTypeName(KQuery::DAY)) {
                filename = getParam<string>(*iter);
                H5FilePtr h5file(new H5::H5File(filename,
                                 H5F_ACC_RDONLY), Hdf5FileCloser());
                m_h5file_map[market+"_DAY"] = h5file;
                m_h5file_map[market+"_WEEK"] = h5file;
                m_h5file_map[market+"_MONTH"] = h5file;
                m_h5file_map[market+"_QUARTER"] = h5file;
                m_h5file_map[market+"_HALFYEAR"] = h5file;
                m_h5file_map[market+"_YEAR"] = h5file;

            } else if (ktype == KQuery::getKTypeName(KQuery::MIN)) {
                filename = getParam<string>(*iter);
                H5FilePtr h5file(new H5::H5File(filename,
                                 H5F_ACC_RDONLY), Hdf5FileCloser());
                m_h5file_map[market+"_MIN"] = h5file;

            } else if (ktype == KQuery::getKTypeName(KQuery::MIN5)) {
                filename = getParam<string>(*iter);
                H5FilePtr h5file(new H5::H5File(filename,
                                 H5F_ACC_RDONLY), Hdf5FileCloser());
                m_h5file_map[market+"_MIN5"] = h5file;
                m_h5file_map[market+"_MIN15"] = h5file;
                m_h5file_map[market+"_MIN30"] = h5file;
                m_h5file_map[market+"_MIN60"] = h5file;
            }

        } catch(...) {
            HKU_ERROR("Can't open h5file: "
                      << filename
                      << " [H5KDataDriver::_init]");
        }
    }

    return true;
}


void H5KDataDriver
::H5ReadRecords(H5::DataSet& dataset, hsize_t start,
                hsize_t nrecords, void *data) {
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t offset[1];
    hsize_t count[1];
    offset[0] = start;
    count[0] = nrecords;
    H5::DataSpace memspace(1, count);
    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
    dataset.read(data, m_h5DataType, memspace, dataspace);
    memspace.close();
    dataspace.close();
    return;
}

void H5KDataDriver
::H5ReadIndexRecords(H5::DataSet& dataset, hsize_t start,
                hsize_t nrecords, void *data) {
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t offset[1];
    hsize_t count[1];
    offset[0] = start;
    count[0] = nrecords;
    H5::DataSpace memspace(1, count);
    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
    dataset.read(data, m_h5IndexType, memspace, dataspace);
    memspace.close();
    dataspace.close();
    return;
}

void H5KDataDriver::loadKData(const string& market, const string& code,
        KQuery::KType kType, size_t start_ix, size_t end_ix,
        KRecordListPtr out_buffer) {
    if (KQuery::INVALID_KTYPE <= kType
            || start_ix >= end_ix) {
        return;
    }

    if (KQuery::DAY == kType
            || KQuery::MIN5 == kType
            || KQuery::MIN == kType) {
        _loadBaseData(market, code, kType, start_ix, end_ix, out_buffer);
        return;
    }

    _loadIndexData(market, code, kType, start_ix, end_ix, out_buffer);
    return;
}

void H5KDataDriver::
_loadBaseData(const string& market, const string& code, KQuery::KType kType,
        size_t start_ix, size_t end_ix, KRecordListPtr out_buffer) {
    H5FilePtr h5file;
    H5::Group group;
    if (!_getH5FileAndGroup(market, code, kType, h5file, group)) {
        return;
    }

    H5Record *pBuf=NULL;
    try{
        string tablename(market + code);
        H5::DataSet dataset(group.openDataSet(tablename));
        H5::DataSpace dataspace = dataset.getSpace();
        size_t all_total = dataspace.getSelectNpoints();
        if (0 == all_total || start_ix >= all_total) {
            return;
        }

        size_t total = end_ix > all_total ? all_total - start_ix : end_ix - start_ix;
        pBuf = new H5Record[total];
        H5ReadRecords(dataset, start_ix, total, pBuf);

        KRecord record;
        out_buffer->reserve(total + 2);
        for(hsize_t i=0; i<total; i++){
            record.datetime = Datetime(pBuf[i].datetime);
            record.openPrice = price_t(pBuf[i].openPrice)*0.001;
            record.highPrice = price_t(pBuf[i].highPrice)*0.001;
            record.lowPrice = price_t(pBuf[i].lowPrice)*0.001;
            record.closePrice = price_t(pBuf[i].closePrice)*0.001;
            record.transAmount = price_t(pBuf[i].transAmount)*0.1;
            record.transCount = price_t(pBuf[i].transCount);
            out_buffer->push_back(record);
        }

    } catch(std::out_of_range& e) {
        HKU_WARN("[H5KDataDriver::_loadBaseData] Invalid date! market_code("
                << market << code << ") "
                << e.what());

    } catch(...) {

    }

    delete [] pBuf;
    return;
}


void H5KDataDriver::
_loadIndexData(const string& market, const string& code,
        KQuery::KType kType, size_t start_ix, size_t end_ix,
        KRecordListPtr out_buffer) {
    string tablename(market + code);
    H5FilePtr h5file;
    H5::Group index_group;
    if (!_getH5FileAndGroup(market, code, kType, h5file, index_group)) {
        return;
    }


    H5Record *p_base_buf = NULL;
    H5IndexRecord *p_index_buf = NULL;
    try{
        H5::Group base_group = h5file->openGroup("data");
        H5::DataSet base_dataset(base_group.openDataSet(tablename));
        H5::DataSpace base_dataspace = base_dataset.getSpace();
        size_t base_total = base_dataspace.getSelectNpoints();
        if (0 == base_total) {
            return;
        }

        H5::DataSet index_dataset(index_group.openDataSet(tablename));
        H5::DataSpace index_dataspace = index_dataset.getSpace();
        size_t index_total = index_dataspace.getSelectNpoints();
        if (0 == index_total || start_ix >= index_total) {
            return;
        }

        size_t index_len = end_ix > index_total ? index_total - start_ix
                         : end_ix - start_ix;
        p_index_buf = new H5IndexRecord[index_len + 1];
        if (end_ix >= index_total) {
            H5ReadIndexRecords(index_dataset, start_ix, index_len, p_index_buf);
            p_index_buf[index_len].start = base_total;
        } else {
            index_len = end_ix - start_ix;
            H5ReadIndexRecords(index_dataset, start_ix, index_len + 1, p_index_buf);
        }

        size_t base_len = p_index_buf[index_len].start - p_index_buf[0].start;
        p_base_buf = new H5Record[base_len];
        H5ReadRecords(base_dataset, p_index_buf[0].start, base_len, p_base_buf);

        KRecord record;
        out_buffer->reserve(index_len);
        for(hsize_t i = 0; i < index_len; i++){
            record.datetime = Datetime(p_index_buf[i].datetime);
            size_t start_pos = p_index_buf[i].start - p_index_buf[0].start;
            size_t end_pos = p_index_buf[i+1].start - p_index_buf[0].start;
            record.openPrice = 0.001 * price_t(p_base_buf[start_pos].openPrice);
            record.closePrice = 0.001 * price_t(p_base_buf[end_pos - 1].closePrice);
            record.highPrice = 0.001 * price_t(p_base_buf[start_pos].highPrice);
            record.lowPrice = 0.001 * price_t(p_base_buf[start_pos].lowPrice);
            record.transAmount = 0.1 * price_t(p_base_buf[start_pos].transAmount);
            record.transCount = price_t(p_base_buf[start_pos].transCount);
            for(hsize_t j = start_pos + 1; j < end_pos; j++){
                price_t temp = 0.001 * price_t(p_base_buf[j].highPrice);
                if( temp > record.highPrice ){
                    record.highPrice = temp;
                }
                temp = 0.001 * price_t(p_base_buf[j].lowPrice);
                if( temp < record.lowPrice ){
                    record.lowPrice = temp;
                }
                record.transAmount += 0.1 * price_t(p_base_buf[j].transAmount);
                record.transCount += p_base_buf[j].transCount;
            }
            out_buffer->push_back(record);
        }

    } catch(std::out_of_range& e) {
        HKU_WARN("[H5KDataDriver::_loadIndexData] Invalid date! " << e.what());

    } catch(...) {

    }

    delete [] p_base_buf;
    delete [] p_index_buf;
    return;
}

bool H5KDataDriver::
_getH5FileAndGroup(const string& market, const string& code,
        KQuery::KType kType, H5FilePtr& out_file, H5::Group& out_group) {
    try {
        string key(market + "_" + KQuery::getKTypeName(kType));
        boost::to_upper(key);

        out_file = m_h5file_map[key];

        if (!out_file) {
            return false;
        }
        switch(kType) {
        case KQuery::MIN:
            out_group = out_file->openGroup("data");
            break;
        case KQuery::MIN5:
            out_group = out_file->openGroup("data");
            break;
        case KQuery::MIN15:
            out_group = out_file->openGroup("min15");
            break;
        case KQuery::MIN30:
            out_group = out_file->openGroup("min30");
            break;
        case KQuery::MIN60:
            out_group = out_file->openGroup("min60");
            break;
        case KQuery::DAY:
            out_group = out_file->openGroup("data");
            break;
        case KQuery::WEEK:
            out_group = out_file->openGroup("week");
            break;
        case KQuery::MONTH:
            out_group = out_file->openGroup("month");
            break;
        case KQuery::QUARTER:
            out_group = out_file->openGroup("quarter");
            break;
        case KQuery::HALFYEAR:
            out_group = out_file->openGroup("halfyear");
            break;
        case KQuery::YEAR:
            out_group = out_file->openGroup("year");
            break;
        default:
            return false;
        }
    } catch(...) {
        HKU_ERROR("[H5KDataDriver::_getH5FileAndGroup] Exception of some HDF5 operator!");
        return false;
    }

    return true;
}

size_t H5KDataDriver
::getCount(const string& market, const string& code, KQuery::KType kType) {
    H5FilePtr h5file;
    H5::Group group;
    if (!_getH5FileAndGroup(market, code, kType, h5file, group)) {
        return 0;
    }

    size_t total=0;

    try{
        H5::DataSet dataset(group.openDataSet(market+code));
        H5::DataSpace dataspace = dataset.getSpace();
        total = dataspace.getSelectNpoints();
        dataspace.close();
        dataset.close();
    }catch(...){
        HKU_WARN("[H5KDataDriver2::getCount] "
              << "Exception of some HDF5 operator! stock: "
              << market << code << " " << KQuery::getKTypeName(kType));
        total = 0;
    }

    return total;
}

KRecord H5KDataDriver
::getKRecord(const string& market, const string& code,
          size_t pos, KQuery::KType kType) {
    string tablename(market + code);

    if (KQuery::DAY == kType || KQuery::MIN5 == kType || KQuery::MIN == kType) {
        return _getBaseRecord(market, code, pos, kType);
    }

    return _getOtherRecord(market, code, pos, kType);
}


KRecord H5KDataDriver::
_getBaseRecord(const string& market, const string& code,
        size_t pos, KQuery::KType kType) {
    assert( KQuery::DAY == kType
            || KQuery::MIN == kType
            || KQuery::MIN5 == kType);

    KRecord result;
    H5FilePtr h5file;
    H5::Group group;
    if (!_getH5FileAndGroup(market, code, kType, h5file, group)) {
        return result;
    }

    H5Record h5record;
    try {
        H5::DataSet dataset(group.openDataSet(market + code));
        H5::DataSpace dataspace = dataset.getSpace();
        hsize_t total = dataspace.getSelectNpoints();
        if(pos >= total) {
            return result;
        }
        H5ReadRecords(dataset, pos, 1, &h5record);

        result.datetime = Datetime(h5record.datetime);
        result.openPrice = price_t(h5record.openPrice) * 0.001;
        result.closePrice = price_t(h5record.closePrice) * 0.001;
        result.lowPrice = price_t(h5record.lowPrice) * 0.001;
        result.highPrice = price_t(h5record.highPrice) * 0.001;
        result.transAmount = price_t(h5record.transAmount) * 0.1;
        result.transCount = price_t(h5record.transCount);

    } catch(std::out_of_range&) {
        HKU_WARN("[H5KDataDriver2::_getBaseRecord] Invalid date!"
               << market << code << "(pos: " << pos << ") datetime: "
               << h5record.datetime);
        result = Null<KRecord>();

    } catch(...) {
        HKU_WARN("[H5KDataDriver2::_getBaseRecord] HDF5(" << market << code
                << "h5) read error!");
        result = Null<KRecord>();
    }

    return result;
}

KRecord H5KDataDriver
::_getOtherRecord(const string& market, const string& code,
        size_t pos, KQuery::KType kType) {
    assert(KQuery::MIN15 == kType
        || KQuery::MIN30    == kType
        || KQuery::MIN60    == kType
        || KQuery::WEEK     == kType
        || KQuery::MONTH    == kType
        || KQuery::QUARTER  == kType
        || KQuery::HALFYEAR == kType
        || KQuery::YEAR     == kType);

    KRecord result;
    H5FilePtr h5file;
    H5::Group group;
    if (!_getH5FileAndGroup(market, code, kType, h5file, group)) {
        return result;
    }

    string _tablename(market + code);
    H5Record *pBuf = NULL;
    try{
        H5::DataSet dataset(group.openDataSet(_tablename));
        H5::DataSpace dataspace = dataset.getSpace();
        hsize_t total = dataspace.getSelectNpoints();
        if(pos >= total) {
            return result;
        }

        H5IndexRecord temp[2];
        if( pos == total-1 ){
            H5ReadIndexRecords(dataset, pos, 1, temp);
            group = h5file->openGroup("data");
            dataset = H5::DataSet(group.openDataSet(_tablename));
            H5::DataSpace dataspace = dataset.getSpace();
            temp[1].start = dataspace.getSelectNpoints();
            dataspace.close();
        }else{
            //如果end不等于total即不包括最后一条记录时，多读取一条记录
            H5ReadIndexRecords(dataset, pos, 2, temp);
            group = h5file->openGroup("data");
            dataset = H5::DataSet(group.openDataSet(_tablename));
        }

        hsize_t recordTotal = temp[1].start - temp[0].start;
        pBuf = new H5Record[recordTotal];
        H5ReadRecords(dataset, temp[0].start, recordTotal, pBuf);

        result.datetime    = Datetime(temp[0].datetime);
        result.openPrice   = 0.001 * price_t(pBuf[0].openPrice);
        result.closePrice  = 0.001 * price_t(pBuf[recordTotal-1].closePrice);
        result.highPrice   = 0.001 * price_t(pBuf[0].highPrice);
        result.lowPrice    = 0.001 * price_t(pBuf[0].lowPrice);
        result.transAmount =  0.1  * price_t(pBuf[0].transAmount);
        result.transCount  =         price_t(pBuf[0].transCount);
        for(hsize_t i=1; i<recordTotal; i++){
            price_t highPrice = 0.001 * price_t(pBuf[i].highPrice);
            if(highPrice > result.highPrice){
                result.highPrice = highPrice;
            }
            price_t lowPrice = 0.001 * price_t(pBuf[i].lowPrice);
            if(lowPrice < result.lowPrice){
                result.lowPrice = lowPrice;
            }
            result.transAmount += 0.1 * price_t(pBuf[i].transAmount);
            result.transCount  +=       price_t(pBuf[i].transCount);
        }

    } catch(std::out_of_range&) {
        HKU_WARN("[H5KDataDriver2::_getOtherRecord] Invalid date!"
                 " stock: " << market << code << " "
                 << KQuery::getKTypeName(kType) << pos);
        result = Null<KRecord>();

    }catch(...){
        HKU_WARN("[H5KDataDriver2::_getOtherRecord] "
                "Exception of some HDF5 operator! stock: "
                << market << code << " "
                << KQuery::getKTypeName(kType) << " " << pos);
        result = Null<KRecord>();
    }

    delete[] pBuf;
    return result;
}


bool H5KDataDriver::
getIndexRangeByDate(const string& market, const string& code, const KQuery& query,
        size_t& out_start, size_t& out_end) {
    assert(KQuery::DATE == query.queryType());

    if (KQuery::MIN5 == query.kType()
            || KQuery::MIN == query.kType()
            || KQuery::DAY == query.kType()) {
        return _getBaseIndexRangeByDate(market, code, query,
                out_start, out_end);
    } else {
        return _getOtherIndexRangeByDate(market, code, query,
                out_start, out_end);
    }

    return false;
}


bool H5KDataDriver::
_getBaseIndexRangeByDate(const string& market, const string& code,
        const KQuery& query, size_t& out_start, size_t& out_end) {
    assert( KQuery::DATE == query.queryType());
    assert( KQuery::DAY  == query.kType()
         || KQuery::MIN  == query.kType()
         || KQuery::MIN5 == query.kType());

    out_start = 0;
    out_end = 0;

    if(query.startDatetime() >= query.endDatetime()
    || query.startDatetime() > Datetime::max()) {
        return false;
    }

    H5FilePtr h5file;
    H5::Group group;
    bool success;
    success = _getH5FileAndGroup(market,code, query.kType(), h5file, group);
    if(!success) {
        return false;
    }

    H5::DataSet dataset;
    H5::DataSpace dataspace;
    hku_uint64 start_number = query.startDatetime().number();
    hku_uint64 end_number = query.endDatetime().number();
    hsize_t startpos = 0, endpos = 0;
    try {
        dataset = group.openDataSet(market + code);
        dataspace = dataset.getSpace();
        hsize_t total = dataspace.getSelectNpoints();
        if (0 == total) {
            return false;
        }

        hsize_t mid, low = 0, high = total - 1;
        H5Record h5record;
        while(low <= high) {
            H5ReadRecords(dataset, high, 1, &h5record);
            if(start_number > h5record.datetime) {
                mid = high + 1;
                break;
            }

            H5ReadRecords(dataset, low, 1, &h5record);
            if(h5record.datetime >= start_number) {
                mid = low;
                break;
            }

            mid = (low + high) / 2;
            H5ReadRecords(dataset, mid, 1, &h5record);
            if(start_number > h5record.datetime) {
                low = mid + 1;
            }else {
                high = mid - 1;
            }
        }

        if(mid >= total) {
            dataspace.close();
            dataset.close();
            return false;
        }

        startpos = mid;

        low = mid;
        high = total - 1;
        while(low <= high){
            H5ReadRecords(dataset, high, 1, &h5record);
            if( end_number > h5record.datetime ){
                mid = high + 1;
                break;
            }

            H5ReadRecords(dataset, low, 1, &h5record);
            if( h5record.datetime >= end_number ){
                mid = low;
                break;
            }

            mid = (low + high)/2;
            H5ReadRecords(dataset, mid, 1, &h5record);
            if( end_number > h5record.datetime){
                low = mid + 1;
            }else{
                high = mid - 1;
            }
        }

        endpos = mid >= total ? total : mid;
        if(startpos >= endpos) {
            dataspace.close();
            dataset.close();
            return false;
        }

        dataspace.close();
        dataset.close();

    } catch(std::out_of_range&) {
        HKU_WARN("[H5KDataDriver2::_getBaseIndexRangeByDate] "
                    "Invalid datetime!");
        dataspace.close();
        dataset.close();
        return false;

    } catch(...) {
        HKU_INFO("[H5KDataDriver2::_getBaseIndexRangeByDate] error in "
                << market << code);
        dataspace.close();
        dataset.close();
    }

    out_start = startpos;
    out_end = endpos;

    return true;
}


bool H5KDataDriver::
_getOtherIndexRangeByDate(const string& market, const string& code,
        const KQuery& query, size_t& out_start, size_t& out_end) {
    assert( KQuery::DATE == query.queryType() );
    assert( KQuery::WEEK     == query.kType()
         || KQuery::MONTH    == query.kType()
         || KQuery::QUARTER  == query.kType()
         || KQuery::HALFYEAR == query.kType()
         || KQuery::YEAR     == query.kType()
         || KQuery::MIN15    == query.kType()
         || KQuery::MIN30    == query.kType()
         || KQuery::MIN60    == query.kType());

    out_start = 0;
    out_end = 0;
    if( query.startDatetime() >= query.endDatetime() ){
        return false;
    }


    H5FilePtr h5file;
    H5::Group group;
    if( ! _getH5FileAndGroup(market, code, query.kType(), h5file, group) ){
        return false;
    }

    string _tablename(market + code);
    size_t total = 0;

    try{
        H5::DataSet dataset(group.openDataSet(_tablename));
        H5::DataSpace dataspace = dataset.getSpace();
        total = dataspace.getSelectNpoints();
        if (0 == total) {
            return false;
        }

        size_t mid, low=0, high=total-1;
        hku_uint64 startDatetime = query.startDatetime().number();
        H5IndexRecord h5record;
        while(low<=high){
            H5ReadIndexRecords(dataset, high, 1, &h5record);
            if(startDatetime > h5record.datetime){
                mid = high + 1;
                break;
            }
            H5ReadIndexRecords(dataset, low, 1, &h5record);
            if(h5record.datetime >= startDatetime){
                mid = low;
                break;
            }

            mid = (low + high)/2;
            H5ReadIndexRecords(dataset, mid, 1, &h5record);
            if( h5record.datetime < startDatetime){
                low = mid + 1;
            }else{
                high = mid - 1;
            }
        }

        if(mid >= total){
            out_start = 0;
            out_end = 0;
            return false;
        }

        out_start = mid;

        hku_uint64 endDatetime = query.endDatetime().number();
        low=mid, high = total-1;
        while(low<=high){
            H5ReadIndexRecords(dataset, high, 1, &h5record);
            if( h5record.datetime < endDatetime ){
                mid = high + 1;
                break;
            }

            H5ReadIndexRecords(dataset, low, 1, &h5record);
            if( h5record.datetime >= endDatetime ){
                mid = low;
                break;
            }

            mid = (low + high)/2;
            H5ReadIndexRecords(dataset, mid, 1, &h5record);
            if( h5record.datetime < endDatetime){
                low = mid + 1;
            }else{
                high = mid - 1;
            }
        }

        out_end = mid >= total ? total : mid;

        if( out_start >= out_end ){
            out_start = 0;
            out_end = 0;
            return false;
        }

    }catch(...){
        out_start = 0;
        out_end = 0;
        return false;
    }

    return true;
}

} /* namespace hku */
