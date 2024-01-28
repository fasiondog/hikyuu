/*
 * H5KDataDriver.cpp
 *
 *  Created on: 2017年10月11日
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "H5KDataDriver.h"

#if H5_VERSION_GE(1, 12, 0)
#define CHECK_DATASET_EXISTS(group, tablename) \
    do {                                       \
        if (!group.exists(tablename)) {        \
            return;                            \
        }                                      \
    } while (0)

#define CHECK_DATASET_EXISTS_RET(group, tablename, ret) \
    do {                                                \
        if (!group.exists(tablename)) {                 \
            return ret;                                 \
        }                                               \
    } while (0)

#define CHECK_GROUP_EXISTS_RET(outfile, name, ret) \
    do {                                           \
        if (!outfile->exists(name)) {              \
            return ret;                            \
        }                                          \
    } while (0)

#else
#define CHECK_DATASET_EXISTS(group, tablename)
#define CHECK_DATASET_EXISTS_RET(group, tablename, ret)
#define CHECK_GROUP_EXISTS_RET(outfile, name, ret)
#endif

namespace hku {

class Hdf5FileCloser {
public:
    void operator()(H5::H5File* h5file) {
        if (h5file) {
            // string filename(h5file->getFileName());
            h5file->close();
            // HKU_INFO("Closed {}", filename);
            delete h5file;
        }
    }
};

H5KDataDriver::H5KDataDriver() : KDataDriver("hdf5"), m_h5DataType(H5::CompType(sizeof(H5Record))) {
    m_h5DataType.insertMember("datetime", HOFFSET(H5Record, datetime), H5::PredType::NATIVE_UINT64);
    m_h5DataType.insertMember("openPrice", HOFFSET(H5Record, openPrice), H5::PredType::NATIVE_UINT);
    m_h5DataType.insertMember("highPrice", HOFFSET(H5Record, highPrice), H5::PredType::NATIVE_UINT);
    m_h5DataType.insertMember("lowPrice", HOFFSET(H5Record, lowPrice), H5::PredType::NATIVE_UINT);
    m_h5DataType.insertMember("closePrice", HOFFSET(H5Record, closePrice),
                              H5::PredType::NATIVE_UINT);
    m_h5DataType.insertMember("transAmount", HOFFSET(H5Record, transAmount),
                              H5::PredType::NATIVE_UINT64);
    m_h5DataType.insertMember("transCount", HOFFSET(H5Record, transCount),
                              H5::PredType::NATIVE_UINT64);

    m_h5IndexType = H5::CompType(sizeof(H5IndexRecord));
    m_h5IndexType.insertMember("datetime", HOFFSET(H5IndexRecord, datetime),
                               H5::PredType::NATIVE_UINT64);
    m_h5IndexType.insertMember("start", HOFFSET(H5IndexRecord, start), H5::PredType::NATIVE_UINT64);

    m_h5TimeLineType = H5::CompType(sizeof(H5TimeLineRecord));
    m_h5TimeLineType.insertMember("datetime", HOFFSET(H5TimeLineRecord, datetime),
                                  H5::PredType::NATIVE_UINT64);
    m_h5TimeLineType.insertMember("price", HOFFSET(H5TimeLineRecord, price),
                                  H5::PredType::NATIVE_UINT64);
    m_h5TimeLineType.insertMember("vol", HOFFSET(H5TimeLineRecord, vol),
                                  H5::PredType::NATIVE_UINT64);

    m_h5TransType = H5::CompType(sizeof(H5TransRecord));
    m_h5TransType.insertMember("datetime", HOFFSET(H5TransRecord, datetime),
                               H5::PredType::NATIVE_UINT64);
    m_h5TransType.insertMember("price", HOFFSET(H5TransRecord, price), H5::PredType::NATIVE_UINT64);
    m_h5TransType.insertMember("vol", HOFFSET(H5TransRecord, vol), H5::PredType::NATIVE_UINT64);
    m_h5TransType.insertMember("buyorsell", HOFFSET(H5TransRecord, buyorsell),
                               H5::PredType::NATIVE_UINT8);
}

H5KDataDriver::~H5KDataDriver() {}

bool H5KDataDriver::_init() {
    // 关闭HDF异常自动打印
    H5::Exception::dontPrint();

    StringList keys = m_params.getNameList();
    string filename;
    for (auto iter = keys.begin(); iter != keys.end(); ++iter) {
        size_t pos = iter->find("_");
        if (pos == string::npos || pos == 0 || pos == iter->size() - 1)
            continue;

        string market = iter->substr(0, pos);
        string ktype = iter->substr(pos + 1);
        to_upper(market);
        to_upper(ktype);

        try {
            if (ktype == KQuery::getKTypeName(KQuery::DAY)) {
                filename = getParam<string>(*iter);
                H5FilePtr h5file(new H5::H5File(filename, H5F_ACC_RDONLY), Hdf5FileCloser());
                m_h5file_map[market + "_DAY"] = h5file;
                m_h5file_map[market + "_WEEK"] = h5file;
                m_h5file_map[market + "_MONTH"] = h5file;
                m_h5file_map[market + "_QUARTER"] = h5file;
                m_h5file_map[market + "_HALFYEAR"] = h5file;
                m_h5file_map[market + "_YEAR"] = h5file;

            } else if (ktype == KQuery::getKTypeName(KQuery::MIN)) {
                filename = getParam<string>(*iter);
                H5FilePtr h5file(new H5::H5File(filename, H5F_ACC_RDONLY), Hdf5FileCloser());
                m_h5file_map[market + "_MIN"] = h5file;

            } else if (ktype == KQuery::getKTypeName(KQuery::MIN5)) {
                filename = getParam<string>(*iter);
                H5FilePtr h5file(new H5::H5File(filename, H5F_ACC_RDONLY), Hdf5FileCloser());
                m_h5file_map[market + "_MIN5"] = h5file;
                m_h5file_map[market + "_MIN15"] = h5file;
                m_h5file_map[market + "_MIN30"] = h5file;
                m_h5file_map[market + "_MIN60"] = h5file;
                m_h5file_map[market + "_HOUR2"] = h5file;

            } else if (ktype == "TIME") {
                filename = getParam<string>(*iter);
                H5FilePtr h5file(new H5::H5File(filename, H5F_ACC_RDONLY), Hdf5FileCloser());
                m_h5file_map[market + "_TIME"] = h5file;

            } else if (ktype == "TRANS") {
                filename = getParam<string>(*iter);
                H5FilePtr h5file(new H5::H5File(filename, H5F_ACC_RDONLY), Hdf5FileCloser());
                m_h5file_map[market + "_TRANS"] = h5file;
            }

        } catch (...) {
            HKU_ERROR("Can't open h5file: {}", filename);
        }
    }

    return true;
}

void H5KDataDriver::H5ReadRecords(H5::DataSet& dataset, hsize_t start, hsize_t nrecords,
                                  void* data) {
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

void H5KDataDriver::H5ReadIndexRecords(H5::DataSet& dataset, hsize_t start, hsize_t nrecords,
                                       void* data) {
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

void H5KDataDriver::H5ReadTimeLineRecords(H5::DataSet& dataset, hsize_t start, hsize_t nrecords,
                                          void* data) {
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t offset[1];
    hsize_t count[1];
    offset[0] = start;
    count[0] = nrecords;
    H5::DataSpace memspace(1, count);
    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
    dataset.read(data, m_h5TimeLineType, memspace, dataspace);
    memspace.close();
    dataspace.close();
    return;
}

void H5KDataDriver::H5ReadTransRecords(H5::DataSet& dataset, hsize_t start, hsize_t nrecords,
                                       void* data) {
    H5::DataSpace dataspace = dataset.getSpace();
    hsize_t offset[1];
    hsize_t count[1];
    offset[0] = start;
    count[0] = nrecords;
    H5::DataSpace memspace(1, count);
    dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);
    dataset.read(data, m_h5TransType, memspace, dataspace);
    memspace.close();
    dataspace.close();
    return;
}

bool H5KDataDriver::_getH5FileAndGroup(const string& market, const string& code,
                                       KQuery::KType kType, H5FilePtr& out_file,
                                       H5::Group& out_group) {
    try {
        string key(format("{}_{}", market, kType));
        to_upper(key);

        out_file = m_h5file_map[key];

        if (!out_file) {
            return false;
        }

        if (kType == KQuery::MIN) {
            CHECK_GROUP_EXISTS_RET(out_file, "data", false);
            out_group = out_file->openGroup("data");
        } else if (kType == KQuery::MIN5) {
            CHECK_GROUP_EXISTS_RET(out_file, "data", false);
            out_group = out_file->openGroup("data");
        } else if (kType == KQuery::MIN15) {
            CHECK_GROUP_EXISTS_RET(out_file, "min15", false);
            out_group = out_file->openGroup("min15");
        } else if (kType == KQuery::MIN30) {
            CHECK_GROUP_EXISTS_RET(out_file, "min30", false);
            out_group = out_file->openGroup("min30");
        } else if (kType == KQuery::MIN60) {
            CHECK_GROUP_EXISTS_RET(out_file, "min60", false);
            out_group = out_file->openGroup("min60");
        } else if (kType == KQuery::HOUR2) {
            CHECK_GROUP_EXISTS_RET(out_file, "hour2", false);
            out_group = out_file->openGroup("hour2");
        } else if (kType == KQuery::DAY) {
            CHECK_GROUP_EXISTS_RET(out_file, "data", false);
            out_group = out_file->openGroup("data");
        } else if (kType == KQuery::WEEK) {
            CHECK_GROUP_EXISTS_RET(out_file, "week", false);
            out_group = out_file->openGroup("week");
        } else if (kType == KQuery::MONTH) {
            CHECK_GROUP_EXISTS_RET(out_file, "month", false);
            out_group = out_file->openGroup("month");
        } else if (kType == KQuery::QUARTER) {
            CHECK_GROUP_EXISTS_RET(out_file, "quarter", false);
            out_group = out_file->openGroup("quarter");
        } else if (kType == KQuery::HALFYEAR) {
            CHECK_GROUP_EXISTS_RET(out_file, "halfyear", false);
            out_group = out_file->openGroup("halfyear");
        } else if (kType == KQuery::YEAR) {
            CHECK_GROUP_EXISTS_RET(out_file, "year", false);
            out_group = out_file->openGroup("year");
        } else if (kType == "TIME") {
            CHECK_GROUP_EXISTS_RET(out_file, "data", false);
            out_group = out_file->openGroup("data");
        } else if (kType == "TRANS") {
            CHECK_GROUP_EXISTS_RET(out_file, "data", false);
            out_group = out_file->openGroup("data");
        } else {
            return false;
        }
    } catch (...) {
        // HKU_ERROR("Exception of some HDF5 operator!");
        return false;
    }

    return true;
}

size_t H5KDataDriver::getCount(const string& market, const string& code,
                               const KQuery::KType& kType) {
    H5FilePtr h5file;
    H5::Group group;
    HKU_IF_RETURN(!_getH5FileAndGroup(market, code, kType, h5file, group), 0);

    size_t total = 0;
    try {
        string tablename(market + code);
        CHECK_DATASET_EXISTS_RET(group, tablename, total);
        H5::DataSet dataset(group.openDataSet(tablename));
        H5::DataSpace dataspace = dataset.getSpace();
        total = dataspace.getSelectNpoints();
        dataspace.close();
        dataset.close();
    } catch (...) {
        // HKU_WARN("Exception of some HDF5 operator! stock: {}{} {}", market, code,
        //         KQuery::getKTypeName(kType));
        total = 0;
    }

    return total;
}

bool H5KDataDriver::getIndexRangeByDate(const string& market, const string& code,
                                        const KQuery& query, size_t& out_start, size_t& out_end) {
    assert(KQuery::DATE == query.queryType());
    bool result = false;
    if (KQuery::MIN5 == query.kType() || KQuery::MIN == query.kType() ||
        KQuery::DAY == query.kType()) {
        result = _getBaseIndexRangeByDate(market, code, query, out_start, out_end);
    } else {
        result = _getOtherIndexRangeByDate(market, code, query, out_start, out_end);
    }

    return result;
}

bool H5KDataDriver::_getBaseIndexRangeByDate(const string& market, const string& code,
                                             const KQuery& query, size_t& out_start,
                                             size_t& out_end) {
    assert(KQuery::DATE == query.queryType());
    assert(KQuery::DAY == query.kType() || KQuery::MIN == query.kType() ||
           KQuery::MIN5 == query.kType());

    out_start = 0;
    out_end = 0;

    if (query.startDatetime() >= query.endDatetime() || query.startDatetime() > Datetime::max()) {
        return false;
    }

    H5FilePtr h5file;
    H5::Group group;
    bool success;
    success = _getH5FileAndGroup(market, code, query.kType(), h5file, group);
    HKU_IF_RETURN(!success, false);

    H5::DataSet dataset;
    H5::DataSpace dataspace;
    hsize_t startpos = 0, endpos = 0;
    try {
        uint64_t start_number = query.startDatetime().number();
        uint64_t end_number = query.endDatetime().number();
        string tablename(market + code);
        CHECK_DATASET_EXISTS_RET(group, tablename, false);
        dataset = group.openDataSet(tablename);
        dataspace = dataset.getSpace();
        hsize_t total = dataspace.getSelectNpoints();
        if (0 == total) {
            return false;
        }

        hsize_t mid, low = 0, high = total - 1;
        H5Record h5record;
        while (low <= high) {
            H5ReadRecords(dataset, high, 1, &h5record);
            if (start_number > h5record.datetime) {
                mid = high + 1;
                break;
            }

            H5ReadRecords(dataset, low, 1, &h5record);
            if (h5record.datetime >= start_number) {
                mid = low;
                break;
            }

            mid = (low + high) / 2;
            H5ReadRecords(dataset, mid, 1, &h5record);
            if (start_number > h5record.datetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        if (mid >= total) {
            dataspace.close();
            dataset.close();
            return false;
        }

        startpos = mid;

        low = mid;
        high = total - 1;
        while (low <= high) {
            H5ReadRecords(dataset, high, 1, &h5record);
            if (end_number > h5record.datetime) {
                mid = high + 1;
                break;
            }

            H5ReadRecords(dataset, low, 1, &h5record);
            if (h5record.datetime >= end_number) {
                mid = low;
                break;
            }

            mid = (low + high) / 2;
            H5ReadRecords(dataset, mid, 1, &h5record);
            if (end_number > h5record.datetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        endpos = mid >= total ? total : mid;
        if (startpos >= endpos) {
            dataspace.close();
            dataset.close();
            return false;
        }

        dataspace.close();
        dataset.close();

    } catch (std::out_of_range&) {
        HKU_WARN("Invalid datetime!");
        dataspace.close();
        dataset.close();
        return false;

    } catch (...) {
        HKU_INFO("error in {}{}", market, code);
        dataspace.close();
        dataset.close();
    }

    out_start = startpos;
    out_end = endpos;

    return true;
}

bool H5KDataDriver::_getOtherIndexRangeByDate(const string& market, const string& code,
                                              const KQuery& query, size_t& out_start,
                                              size_t& out_end) {
    assert(KQuery::DATE == query.queryType());
    assert(KQuery::WEEK == query.kType() || KQuery::MONTH == query.kType() ||
           KQuery::QUARTER == query.kType() || KQuery::HALFYEAR == query.kType() ||
           KQuery::YEAR == query.kType() || KQuery::MIN15 == query.kType() ||
           KQuery::MIN30 == query.kType() || KQuery::MIN60 == query.kType() ||
           KQuery::HOUR2 == query.kType());
    out_start = 0;
    out_end = 0;
    HKU_IF_RETURN(query.startDatetime() >= query.endDatetime(), false);

    H5FilePtr h5file;
    H5::Group group;
    HKU_IF_RETURN(!_getH5FileAndGroup(market, code, query.kType(), h5file, group), false);

    string _tablename(market + code);
    try {
        CHECK_DATASET_EXISTS_RET(group, _tablename, false);
        H5::DataSet dataset(group.openDataSet(_tablename));
        H5::DataSpace dataspace = dataset.getSpace();
        size_t total = dataspace.getSelectNpoints();
        if (0 == total) {
            return false;
        }

        size_t mid = total, low = 0, high = total - 1;
        uint64_t startDatetime = query.startDatetime().number();
        H5IndexRecord h5record;
        while (low <= high) {
            H5ReadIndexRecords(dataset, high, 1, &h5record);
            if (startDatetime > h5record.datetime) {
                mid = high + 1;
                break;
            }
            H5ReadIndexRecords(dataset, low, 1, &h5record);
            if (h5record.datetime >= startDatetime) {
                mid = low;
                break;
            }

            mid = (low + high) / 2;
            H5ReadIndexRecords(dataset, mid, 1, &h5record);
            if (h5record.datetime < startDatetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        if (mid >= total) {
            out_start = 0;
            out_end = 0;
            return false;
        }

        out_start = mid;

        uint64_t endDatetime = query.endDatetime().number();
        low = mid, high = total - 1;
        while (low <= high) {
            H5ReadIndexRecords(dataset, high, 1, &h5record);
            if (h5record.datetime < endDatetime) {
                mid = high + 1;
                break;
            }

            H5ReadIndexRecords(dataset, low, 1, &h5record);
            if (h5record.datetime >= endDatetime) {
                mid = low;
                break;
            }

            mid = (low + high) / 2;
            H5ReadIndexRecords(dataset, mid, 1, &h5record);
            if (h5record.datetime < endDatetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        out_end = mid >= total ? total : mid;

        if (out_start >= out_end) {
            out_start = 0;
            out_end = 0;
            return false;
        }

    } catch (...) {
        out_start = 0;
        out_end = 0;
        return false;
    }

    return true;
}

KRecordList H5KDataDriver::getKRecordList(const string& market, const string& code,
                                          const KQuery& query) {
    KRecordList result;
    auto kType = query.kType();
    if (query.queryType() == KQuery::INDEX) {
        // 按索引方式查询
        HKU_IF_RETURN(query.start() >= query.end(), result);
        if (KQuery::DAY == kType || KQuery::MIN5 == kType || KQuery::MIN == kType) {
            result = _getBaseKRecordList(market, code, kType, query.start(), query.end());
        } else {
            result = _getIndexKRecordList(market, code, kType, query.start(), query.end());
        }

    } else {
        // 按日期方式查询
        size_t out_start = 0, out_end = 0;
        if (KQuery::DAY == kType || KQuery::MIN5 == kType || KQuery::MIN == kType) {
            if (_getBaseIndexRangeByDate(market, code, query, out_start, out_end)) {
                result = _getBaseKRecordList(market, code, kType, out_start, out_end);
            }
        } else {
            if (_getOtherIndexRangeByDate(market, code, query, out_start, out_end)) {
                result = _getIndexKRecordList(market, code, kType, out_start, out_end);
            }
        }
    }

    return result;
}

KRecordList H5KDataDriver::_getBaseKRecordList(const string& market, const string& code,
                                               const KQuery::KType& kType, size_t start_ix,
                                               size_t end_ix) {
    KRecordList result;
    H5FilePtr h5file;
    H5::Group group;
    HKU_IF_RETURN(!_getH5FileAndGroup(market, code, kType, h5file, group), result);

    try {
        string tablename(format("{}{}", market, code));
        CHECK_DATASET_EXISTS_RET(group, tablename, result);
        H5::DataSet dataset(group.openDataSet(tablename));
        H5::DataSpace dataspace = dataset.getSpace();
        size_t all_total = dataspace.getSelectNpoints();
        if (0 == all_total || start_ix >= all_total) {
            return result;
        }

        size_t total = end_ix > all_total ? all_total - start_ix : end_ix - start_ix;
        std::unique_ptr<H5Record[]> pBuf = std::make_unique<H5Record[]>(total);
        H5ReadRecords(dataset, start_ix, total, pBuf.get());

        KRecord record;
        result.reserve(total + 2);
        for (hsize_t i = 0; i < total; i++) {
            record.datetime = Datetime(pBuf[i].datetime);
            record.openPrice = price_t(pBuf[i].openPrice) * 0.001;
            record.highPrice = price_t(pBuf[i].highPrice) * 0.001;
            record.lowPrice = price_t(pBuf[i].lowPrice) * 0.001;
            record.closePrice = price_t(pBuf[i].closePrice) * 0.001;
            record.transAmount = price_t(pBuf[i].transAmount) * 0.1;
            record.transCount = price_t(pBuf[i].transCount);
            result.push_back(record);
        }

    } catch (std::out_of_range& e) {
        HKU_WARN("Invalid date! market_code({}{}) {}", market, code, e.what());

    } catch (std::exception& e) {
        HKU_WARN(e.what());

    } catch (...) {
        // 忽略
    }

    return result;
}

KRecordList H5KDataDriver::_getIndexKRecordList(const string& market, const string& code,
                                                const KQuery::KType& kType, size_t start_ix,
                                                size_t end_ix) {
    KRecordList result;
    string tablename(format("{}{}", market, code));
    H5FilePtr h5file;
    H5::Group index_group;
    HKU_IF_RETURN(!_getH5FileAndGroup(market, code, kType, h5file, index_group), result);

    try {
        H5::Group base_group = h5file->openGroup("data");
        CHECK_DATASET_EXISTS_RET(base_group, tablename, result);
        H5::DataSet base_dataset(base_group.openDataSet(tablename));
        H5::DataSpace base_dataspace = base_dataset.getSpace();
        size_t base_total = base_dataspace.getSelectNpoints();
        if (0 == base_total) {
            return result;
        }

        CHECK_DATASET_EXISTS_RET(index_group, tablename, result);
        H5::DataSet index_dataset(index_group.openDataSet(tablename));
        H5::DataSpace index_dataspace = index_dataset.getSpace();
        size_t index_total = index_dataspace.getSelectNpoints();
        if (0 == index_total || start_ix >= index_total) {
            return result;
        }

        size_t index_len = end_ix > index_total ? index_total - start_ix : end_ix - start_ix;
        std::unique_ptr<H5IndexRecord[]> p_index_buf =
          std::make_unique<H5IndexRecord[]>(index_len + 1);
        if (end_ix >= index_total) {
            H5ReadIndexRecords(index_dataset, start_ix, index_len, p_index_buf.get());
            p_index_buf[index_len].start = base_total;
        } else {
            index_len = end_ix - start_ix;
            H5ReadIndexRecords(index_dataset, start_ix, index_len + 1, p_index_buf.get());
        }

        size_t base_len = p_index_buf[index_len].start - p_index_buf[0].start;
        std::unique_ptr<H5Record[]> p_base_buf = std::make_unique<H5Record[]>(base_len);
        H5ReadRecords(base_dataset, p_index_buf[0].start, base_len, p_base_buf.get());

        KRecord record;
        result.reserve(index_len);
        for (hsize_t i = 0; i < index_len; i++) {
            record.datetime = Datetime(p_index_buf[i].datetime);
            size_t start_pos = p_index_buf[i].start - p_index_buf[0].start;
            size_t end_pos = p_index_buf[i + 1].start - p_index_buf[0].start;
            record.openPrice = 0.001 * price_t(p_base_buf[start_pos].openPrice);
            record.closePrice = 0.001 * price_t(p_base_buf[end_pos - 1].closePrice);
            record.highPrice = 0.001 * price_t(p_base_buf[start_pos].highPrice);
            record.lowPrice = 0.001 * price_t(p_base_buf[start_pos].lowPrice);
            record.transAmount = 0.1 * price_t(p_base_buf[start_pos].transAmount);
            record.transCount = price_t(p_base_buf[start_pos].transCount);
            for (hsize_t j = start_pos + 1; j < end_pos; j++) {
                price_t temp = 0.001 * price_t(p_base_buf[j].highPrice);
                if (temp > record.highPrice) {
                    record.highPrice = temp;
                }
                temp = 0.001 * price_t(p_base_buf[j].lowPrice);
                if (temp < record.lowPrice) {
                    record.lowPrice = temp;
                }
                record.transAmount += 0.1 * price_t(p_base_buf[j].transAmount);
                record.transCount += p_base_buf[j].transCount;
            }
            result.push_back(record);
        }

    } catch (std::out_of_range& e) {
        HKU_WARN("Invalid date! {}", e.what());

    } catch (...) {
        // 忽略
    }

    return result;
}

TimeLineList H5KDataDriver::getTimeLineList(const string& market, const string& code,
                                            const KQuery& query) {
    return query.queryType() == KQuery::INDEX
             ? _getTimeLine(market, code, query.start(), query.end())
             : _getTimeLine(market, code, query.startDatetime(), query.endDatetime());
}

TimeLineList H5KDataDriver::_getTimeLine(const string& market, const string& code, int64_t start_ix,
                                         int64_t end_ix) {
    TimeLineList result;
    H5FilePtr h5file;
    H5::Group group;
    HKU_IF_RETURN(!_getH5FileAndGroup(market, code, "TIME", h5file, group), result);

    try {
        string tablename(market + code);
        CHECK_DATASET_EXISTS_RET(group, tablename, result);
        H5::DataSet dataset(group.openDataSet(tablename));
        H5::DataSpace dataspace = dataset.getSpace();
        size_t all_total = dataspace.getSelectNpoints();
        if (0 == all_total) {
            return result;
        }

        if (start_ix < 0) {
            start_ix += all_total;
            if (start_ix < 0)
                start_ix = 0;
        }

        if (end_ix < 0) {
            end_ix += all_total;
            if (end_ix < 0)
                end_ix = 0;
        }

        if (start_ix >= end_ix) {
            return result;
        }

        size_t startpos = boost::numeric_cast<size_t>(start_ix);
        if (all_total <= startpos) {
            return result;
        }

        size_t endpos = boost::numeric_cast<size_t>(end_ix);
        if (endpos > all_total) {
            endpos = all_total;
        }

        size_t total = endpos - startpos;
        std::unique_ptr<H5TimeLineRecord[]> pBuf = std::make_unique<H5TimeLineRecord[]>(total);
        H5ReadTimeLineRecords(dataset, start_ix, total, pBuf.get());

        TimeLineRecord record;
        result.reserve(total + 2);
        for (hsize_t i = 0; i < total; i++) {
            record.datetime = Datetime(pBuf[i].datetime);
            record.price = price_t(pBuf[i].price) * 0.001;
            record.vol = price_t(pBuf[i].vol);
            result.push_back(record);
        }

    } catch (std::out_of_range& e) {
        HKU_WARN("Invalid date! market_code({}{} {}", market, code, e.what());

    } catch (std::exception& e) {
        HKU_WARN(e.what());

    } catch (...) {
        HKU_WARN("Unkown error!");
    }

    return result;
}

TimeLineList H5KDataDriver::_getTimeLine(const string& market, const string& code,
                                         const Datetime& start, const Datetime& end) {
    TimeLineList result;
    HKU_IF_RETURN(start >= end || start > Datetime::max(), result);

    H5FilePtr h5file;
    H5::Group group;
    bool success;
    success = _getH5FileAndGroup(market, code, "TIME", h5file, group);
    HKU_IF_RETURN(!success, result);

    H5::DataSet dataset;
    H5::DataSpace dataspace;
    hsize_t startpos = 0, endpos = 0;
    try {
        uint64_t start_number = start.number();
        uint64_t end_number = end.number();
        string tablename(market + code);
        CHECK_DATASET_EXISTS_RET(group, tablename, result);
        dataset = group.openDataSet(tablename);
        dataspace = dataset.getSpace();
        hsize_t total = dataspace.getSelectNpoints();
        if (0 == total) {
            return result;
        }

        hsize_t mid, low = 0, high = total - 1;
        H5TimeLineRecord h5record;
        while (low <= high) {
            H5ReadTimeLineRecords(dataset, high, 1, &h5record);
            if (start_number > h5record.datetime) {
                mid = high + 1;
                break;
            }

            H5ReadTimeLineRecords(dataset, low, 1, &h5record);
            if (h5record.datetime >= start_number) {
                mid = low;
                break;
            }

            mid = (low + high) / 2;
            H5ReadTimeLineRecords(dataset, mid, 1, &h5record);
            if (start_number > h5record.datetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        if (mid >= total) {
            dataspace.close();
            dataset.close();
            return result;
        }

        startpos = mid;

        low = mid;
        high = total - 1;
        while (low <= high) {
            H5ReadTimeLineRecords(dataset, high, 1, &h5record);
            if (end_number > h5record.datetime) {
                mid = high + 1;
                break;
            }

            H5ReadTimeLineRecords(dataset, low, 1, &h5record);
            if (h5record.datetime >= end_number) {
                mid = low;
                break;
            }

            mid = (low + high) / 2;
            H5ReadTimeLineRecords(dataset, mid, 1, &h5record);
            if (end_number > h5record.datetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        endpos = mid >= total ? total : mid;
        if (startpos >= endpos) {
            dataspace.close();
            dataset.close();
            return result;
        }

        dataspace.close();
        dataset.close();

    } catch (std::out_of_range&) {
        HKU_WARN("Invalid datetime!");
        dataspace.close();
        dataset.close();
        return result;

    } catch (...) {
        HKU_INFO("error in {}{}", market, code);
        dataspace.close();
        dataset.close();
        return result;
    }

    if (startpos >= endpos) {
        return result;
    }

    try {
        string tablename(market + code);
        CHECK_DATASET_EXISTS_RET(group, tablename, result);
        dataset = group.openDataSet(tablename);
        size_t total = endpos - startpos;
        std::unique_ptr<H5TimeLineRecord[]> pBuf = std::make_unique<H5TimeLineRecord[]>(total);
        H5ReadTimeLineRecords(dataset, startpos, total, pBuf.get());

        TimeLineRecord record;
        result.reserve(total + 2);
        for (hsize_t i = 0; i < total; i++) {
            record.datetime = Datetime(pBuf[i].datetime);
            record.price = price_t(pBuf[i].price) * 0.001;
            record.vol = price_t(pBuf[i].vol);
            result.push_back(record);
        }

    } catch (std::out_of_range& e) {
        HKU_WARN("Invalid date! market_code({}{}) {}", market, code, e.what());

    } catch (std::exception& e) {
        HKU_WARN(e.what());

    } catch (...) {
        HKU_WARN("Unkown error!");
    }

    return result;
}

TransList H5KDataDriver::getTransList(const string& market, const string& code,
                                      const KQuery& query) {
    return query.queryType() == KQuery::INDEX
             ? _getTransList(market, code, query.start(), query.end())
             : _getTransList(market, code, query.startDatetime(), query.endDatetime());
}

TransList H5KDataDriver::_getTransList(const string& market, const string& code, int64_t start_ix,
                                       int64_t end_ix) {
    TransList result;
    H5FilePtr h5file;
    H5::Group group;
    HKU_IF_RETURN(!_getH5FileAndGroup(market, code, "TRANS", h5file, group), result);

    try {
        string tablename(market + code);
        CHECK_DATASET_EXISTS_RET(group, tablename, result);
        H5::DataSet dataset(group.openDataSet(tablename));
        H5::DataSpace dataspace = dataset.getSpace();
        size_t all_total = dataspace.getSelectNpoints();
        if (0 == all_total) {
            return result;
        }

        if (start_ix < 0) {
            start_ix += all_total;
            if (start_ix < 0)
                start_ix = 0;
        }

        if (end_ix < 0) {
            end_ix += all_total;
            if (end_ix < 0)
                end_ix = 0;
        }

        if (start_ix >= end_ix) {
            return result;
        }

        size_t startpos = boost::numeric_cast<size_t>(start_ix);
        if (all_total <= startpos) {
            return result;
        }

        size_t endpos = boost::numeric_cast<size_t>(end_ix);
        if (endpos > all_total) {
            endpos = all_total;
        }

        size_t total = endpos - startpos;
        std::unique_ptr<H5TransRecord[]> pBuf = std::make_unique<H5TransRecord[]>(total);
        H5ReadTransRecords(dataset, start_ix, total, pBuf.get());

        TransRecord record;
        result.reserve(total + 2);
        for (hsize_t i = 0; i < total; i++) {
            uint64_t number = pBuf[i].datetime / 100;
            uint64_t second = pBuf[i].datetime - number * 100;
            Datetime d(number);
            record.datetime =
              Datetime(d.year(), d.month(), d.day(), d.hour(), d.minute(), (long)second);
            record.price = price_t(pBuf[i].price) * 0.001;
            record.vol = price_t(pBuf[i].vol);
            record.direct = TransRecord::DIRECT(pBuf[i].buyorsell);
            result.push_back(record);
        }

    } catch (std::out_of_range& e) {
        HKU_WARN("Invalid date! market_code({}{}) {}", market, code, e.what());

    } catch (boost::bad_numeric_cast&) {
        HKU_WARN("You may be use 32bit system, but data is to larger!");

    } catch (std::exception& e) {
        HKU_ERROR(e.what());

    } catch (...) {
        HKU_ERROR("Unkown error!");
    }

    return result;
}

TransList H5KDataDriver::_getTransList(const string& market, const string& code,
                                       const Datetime& start, const Datetime& end) {
    TransList result;
    HKU_IF_RETURN(start >= end || start > Datetime::max(), result);

    H5FilePtr h5file;
    H5::Group group;
    bool success;
    success = _getH5FileAndGroup(market, code, "TRANS", h5file, group);
    HKU_IF_RETURN(!success, result);

    H5::DataSet dataset;
    H5::DataSpace dataspace;
    uint64_t start_number = start.number() * 100 + start.second();
    uint64_t end_number = end.number() * 100 + (end.isNull() ? 0 : end.second());
    hsize_t startpos = 0, endpos = 0;
    try {
        string tablename(market + code);
        CHECK_DATASET_EXISTS_RET(group, tablename, result);
        dataset = group.openDataSet(tablename);
        dataspace = dataset.getSpace();
        hsize_t total = dataspace.getSelectNpoints();
        if (0 == total) {
            return result;
        }

        hsize_t mid, low = 0, high = total - 1;
        H5TransRecord h5record;
        while (low <= high) {
            H5ReadTransRecords(dataset, high, 1, &h5record);
            if (start_number > h5record.datetime) {
                mid = high + 1;
                break;
            }

            H5ReadTransRecords(dataset, low, 1, &h5record);
            if (h5record.datetime >= start_number) {
                mid = low;
                break;
            }

            mid = (low + high) / 2;
            H5ReadTransRecords(dataset, mid, 1, &h5record);
            if (start_number > h5record.datetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        if (mid >= total) {
            dataspace.close();
            dataset.close();
            return result;
        }

        startpos = mid;

        low = mid;
        high = total - 1;
        while (low <= high) {
            H5ReadTransRecords(dataset, high, 1, &h5record);
            if (end_number > h5record.datetime) {
                mid = high + 1;
                break;
            }

            H5ReadTransRecords(dataset, low, 1, &h5record);
            if (h5record.datetime >= end_number) {
                mid = low;
                break;
            }

            mid = (low + high) / 2;
            H5ReadTransRecords(dataset, mid, 1, &h5record);
            if (end_number > h5record.datetime) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        endpos = mid >= total ? total : mid;
        if (startpos >= endpos) {
            dataspace.close();
            dataset.close();
            return result;
        }

        dataspace.close();
        dataset.close();

    } catch (std::out_of_range&) {
        HKU_WARN("Invalid datetime!");
        dataspace.close();
        dataset.close();
        return result;

    } catch (...) {
        HKU_INFO("error in {}{}", market, code);
        dataspace.close();
        dataset.close();
        return result;
    }

    if (startpos == endpos) {
        return result;
    }

    try {
        string tablename(market + code);
        CHECK_DATASET_EXISTS_RET(group, tablename, result);
        dataset = group.openDataSet(tablename);

        size_t total = endpos - startpos;
        std::unique_ptr<H5TransRecord[]> pBuf = std::make_unique<H5TransRecord[]>(total);
        H5ReadTransRecords(dataset, startpos, total, pBuf.get());

        TransRecord record;
        result.reserve(total + 2);
        for (hsize_t i = 0; i < total; i++) {
            uint64_t number = pBuf[i].datetime / 100;
            uint64_t second = pBuf[i].datetime - number * 100;
            Datetime d(number);
            record.datetime =
              Datetime(d.year(), d.month(), d.day(), d.hour(), d.minute(), (long)second);
            record.price = price_t(pBuf[i].price) * 0.001;
            record.vol = price_t(pBuf[i].vol);
            record.direct = TransRecord::DIRECT(pBuf[i].buyorsell);
            result.push_back(record);
        }

    } catch (std::out_of_range& e) {
        HKU_WARN("Invalid date! market_code({}{}) {}", market, code, e.what());

    } catch (std::exception& e) {
        HKU_WARN(e.what());

    } catch (...) {
        HKU_WARN("Unkown error!");
    }

    return result;
}

} /* namespace hku */
