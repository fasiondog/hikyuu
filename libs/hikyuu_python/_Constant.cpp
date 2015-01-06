/*
 * _Constant.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/DataType.h>
#include "pickle_support.h"

using namespace boost::python;
using namespace hku;

struct Constant {
    Constant():
        null_datetime(Null<Datetime>()),
        null_price(Null<price_t>()),
        null_int(Null<int>()),
        null_size(Null<size_t>()),
        null_int64(Null<hku_int64>()){
#if HKU_PYTHON_SUPPORT_PICKLE
        pickle_support = true;
#else
        pickle_support = false;
#endif /* HKU_PYTHON_SUPPORT_PICKLE */
    }

    Datetime null_datetime;
    price_t null_price;
    int null_int;
    size_t null_size;
    hku_int64 null_int64;
    bool pickle_support; //是否支持pickle
};

void export_Constant() {
    class_<Constant>("Constant")
            .def_readonly("null_datetime", &Constant::null_datetime)
            .def_readonly("null_price", &Constant::null_price)
            .def_readonly("null_int", &Constant::null_int)
            .def_readonly("null_size", &Constant::null_size)
            .def_readonly("null_int64", &Constant::null_int64)
            .def_readonly("pickle_support", &Constant::pickle_support)
            ;
}

