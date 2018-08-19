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
    KDataDriverWrap(const string& name): KDataDriver(name) {}
    virtual ~KDataDriverWrap() {}

    bool _init() {
        if (override call = get_override("_init")) {
            return call();
        } else {
            return KDataDriver::_init();
        }
    }

    bool default_init() {
        return this->KDataDriver::_init();
    }

    void loadKData(const string& market, const string& code,
                KQuery::KType ktype, size_t start_ix, size_t end_ix,
                KRecordListPtr out_buffer) {
        if (override call = get_override("loadKData")) {
            call(market, code, ktype, start_ix, end_ix, out_buffer);
        } else {
            KDataDriver::loadKData(market, code, ktype,
                                   start_ix, end_ix, out_buffer);
        }
    }

    void default_loadKData(const string& market, const string& code,
                KQuery::KType ktype, size_t start_ix, size_t end_ix,
                KRecordListPtr out_buffer) {
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

    virtual object _getIndexRangeByDate(const string& market,
            const string& code, const KQuery& query) {
        if (override call = get_override("_getIndexRangeByDate")) {
            return call(market, code, query);
        }

        return make_tuple(0, 0);
    }

    object default_getIndexRangeByDate(const string& market,
                const string& code, const KQuery& query) {
        return make_tuple(0, 0);
    }

    bool getIndexRangeByDate(const string& market, const string& code,
            const KQuery& query, size_t& out_start, size_t& out_end) {
        out_start = 0;
        out_end = 0;

        object x = _getIndexRangeByDate(market, code, query);
        if (x.is_none() || len(x) < 2) {
            return false;
        }

        extract<size_t> start(x[0]);
        if (start.check()) {
            out_start = start();
        } else {
            return false;
        }

        extract<size_t> end(x[1]);
        if (end.check()) {
            out_end = end();
        } else {
            out_start = 0;
            return false;
        }

        return true;
    }

    /* Python 无法 使用指针或引用作为输出参数
    bool getIndexRangeByDate(const string& market, const string& code,
                const KQuery& query, size_t& out_start, size_t& out_end) {
        if (override call = get_override("getIndexRangeByDate")) {
            return call(market, code, query, out_start, out_end);
        } else {
            return KDataDriver::getIndexRangeByDate(market, code,
                                         query, out_start, out_end);
        }
    }*/

    /*bool default_getIndexRangeByDate(const string& market, const string& code,
                    const KQuery& query, size_t& out_start, size_t& out_end) {
        return this->KDataDriver::getIndexRangeByDate(market, code,
                query, out_start, out_end);
    }*/

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
            .def(init<const string&>())
            .def(self_ns::str(self))
            .add_property("name", make_function(&KDataDriver::name,
                            return_value_policy<copy_const_reference>()))
            .def("getParam", &KDataDriver::getParam<boost::any>)

            .def("_init", &KDataDriver::_init, &KDataDriverWrap::default_init)
            .def("loadKData", &KDataDriver::loadKData,
                    &KDataDriverWrap::default_loadKData)
            .def("getCount", &KDataDriver::getCount,
                    &KDataDriverWrap::default_getCount)
            //.def("getIndexRangeByDate", &KDataDriver::getIndexRangeByDate,
            //        &KDataDriverWrap::default_getIndexRangeByDate)
            .def("getKRecord", &KDataDriver::getKRecord,
                    &KDataDriverWrap::default_getKRecord)
            .def("_getIndexRangeByDate",
                    &KDataDriverWrap::_getIndexRangeByDate,
                    &KDataDriverWrap::default_getIndexRangeByDate)
            ;

    register_ptr_to_python<KDataDriverPtr>();
}


