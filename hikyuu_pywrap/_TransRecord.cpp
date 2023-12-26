/*
 * _KRecord.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <hikyuu/serialization/TransRecord_serialization.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

static std::string TransRecord_to_str(const TransRecord& record) {
    std::stringstream out;
    out << record;
    return out.str();
}

void export_TransRecord(py::module& m) {
    py::class_<TransRecord>(m, "TransRecord")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t, price_t, TransRecord::DIRECT>())
      .def("__str__", TransRecord_to_str)
      .def("__repr__", TransRecord_to_str)
      .def_readwrite("date", &TransRecord::datetime, "时间")
      .def_readwrite("price", &TransRecord::price, "价格")
      .def_readwrite("vol", &TransRecord::vol, "成交量")
      .def_readwrite("direct", &TransRecord::direct, "买卖盘性质, 参见: TransRecord.DIRECT")
      .def(py::self == py::self)

        DEF_PICKLE(TransRecord);

    py::enum_<TransRecord::DIRECT>(m, "DIRECT")
      .value("BUY", TransRecord::BUY)
      .value("SELL", TransRecord::SELL)
      .value("AUCTION", TransRecord::AUCTION);

    //     class_<TransList>("TransList")
    //       .def(vector_indexing_suite<TransList>())
    // #if HKU_PYTHON_SUPPORT_PICKLE
    //       .def_pickle(normal_pickle_suite<TransList>())
    // #endif
    //       ;
}
