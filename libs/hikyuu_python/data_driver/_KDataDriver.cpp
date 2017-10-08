/*
 * _DataDriverFactory.cpp
 *
 *  Created on: 2017年10月7日
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/data_driver/KDataDriver.h>

using namespace hku;
using namespace boost::python;

class KDataDriverWrap: public KDataDriver, public wrapper<KDataDriver> {
public:
    KDataDriverWrap(): KDataDriver() {}

    void loadKData(const string& market, const string& code,
                KQuery::KType ktype, size_t start_ix, size_t end_ix,
                KRecordList* out_buffer) {
        if (override call = get_override("loadKData")) {
            call(market, code, ktype, ktype, start_ix, end_ix, out_buffer);
        } else {
            KDataDriver::loadKData(market, code, ktype,
                                   start_ix, end_ix, out_buffer);
        }
    }

    void default_loadKData(const string& market, const string& code,
                KQuery::KType ktype, size_t start_ix, size_t end_ix,
                KRecordList* out_buffer) {
        this->KDataDriver::loadKData(market, code, ktype,
                start_ix, end_ix, out_buffer);
    }

    size_t getCount(const string& market, const string& code,
                KQuery::KType ktype) {
        if (override call = get_override("getCount")) {
            return call(market, code, ktype);
        } else {
            return KDataDriver::getCount(market, code, ktype);
        }
    }

    size_t default_getCount(const string& market, const string& code,
            KQuery::KType ktype) {
        return this->KDataDriver::getCount(market, code, ktype);
    }

    bool getIndexRangeByDate(const string& market, const string& code,
                const KQuery& query, size_t& out_start, size_t& out_end) {
        if (override call = get_override("getIndexRangeByDate")) {
            return call(market, code, query, out_start, out_end);
        } else {
            return KDataDriver::getIndexRangeByDate(market, code,
                                         query, out_start, out_end);
        }
    }

    bool default_getIndexRangeByDate(const string& market, const string& code,
                    const KQuery& query, size_t& out_start, size_t& out_end) {
        return this->KDataDriver::getIndexRangeByDate(market, code,
                query, out_start, out_end);
    }

    KRecord getKRecord(const string& market, const string& code,
                  size_t pos, KQuery::KType ktype) {
        if (override call = get_override("getKRecord")) {
            return call(market, code, pos, ktype);
        } else {
            return KDataDriver::getKRecord(market, code, pos, ktype);
        }
    }

    KRecord default_getKRecord(const string& market, const string& code,
                      size_t pos, KQuery::KType ktype) {
        return this->KDataDriver::getKRecord(market, code, pos, ktype);
    }

};

void export_KDataDriver() {

    class_<KDataDriverWrap, boost::noncopyable>("KDataDriver", init<>())
            .def("loadKData", &KDataDriver::loadKData,
                    &KDataDriverWrap::default_loadKData)
            .def("getCount", &KDataDriver::getCount,
                    &KDataDriverWrap::default_getCount)
            .def("getIndexRangeByDate", &KDataDriver::getIndexRangeByDate,
                    &KDataDriverWrap::default_getIndexRangeByDate)
            .def("getKRecord", &KDataDriver::getKRecord,
                    &KDataDriverWrap::default_getKRecord)
            ;

    register_ptr_to_python<KDataDriverPtr>();
}


