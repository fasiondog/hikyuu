/*
 * _KRecord.cpp
 *
 *  Created on: 2019-1-27
 *      Author: fasiondog
 */

#include <hikyuu/serialization/TimeLineRecord_serialization.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_TimeLineReord(py::module& m) {
    py::class_<TimeLineRecord>(m, "TimeLineRecord", "分时线记录，属性可读写")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t, price_t>())
      .def("__str__", to_py_str<TimeLineRecord>)
      .def("__repr__", to_py_str<TimeLineRecord>)
      .def_readwrite("date", &TimeLineRecord::datetime, "日期时间")
      .def_readwrite("price", &TimeLineRecord::price, "价格")
      .def_readwrite("vol", &TimeLineRecord::vol, "成交量")
      .def(py::self == py::self)

        DEF_PICKLE(TimeLineRecord);

    m.def(
      "timeline_to_np",
      [](const TimeLineList& timeline) {
          struct RawData {
              int64_t datetime;  // 转换后的毫秒时间戳
              double price;
              double vol;
          };

          std::vector<RawData> data;
          data.resize(timeline.size());
          for (size_t i = 0; i < timeline.size(); i++) {
              const TimeLineRecord& record = timeline[i];
              data[i].datetime = record.datetime.timestamp() / 1000;
              data[i].price = record.price;
              data[i].vol = record.vol;
          }

          // 定义NumPy结构化数据类型
          auto dtype = py::dtype(vector_to_python_list<string>({"datetime", "price", "vol"}),
                                 vector_to_python_list<string>({"datetime64[ms]", "d", "d"}),
                                 vector_to_python_list<int64_t>({0, 8, 16}), 24);

          return py::array(dtype, data.size(), data.data());
      },
      "将分时线记录转换为NumPy元组");
}
