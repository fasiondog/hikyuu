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
          size_t total = timeline.size();
          HKU_IF_RETURN(total == 0, py::array());

          struct RawData {
              int64_t datetime;  // 转换后的毫秒时间戳
              double price;
              double vol;
          };

          // 使用 malloc 分配内存
          RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
          for (size_t i = 0, len = timeline.size(); i < len; i++) {
              const TimeLineRecord& record = timeline[i];
              data[i].datetime = record.datetime.timestamp() * 1000LL;
              data[i].price = record.price;
              data[i].vol = record.vol;
          }

          // 定义NumPy结构化数据类型
          auto dtype = py::dtype(vector_to_python_list<string>({"datetime", "price", "vol"}),
                                 vector_to_python_list<string>({"datetime64[ns]", "d", "d"}),
                                 vector_to_python_list<int64_t>({0, 8, 16}), 24);

          // 使用 capsule 管理内存
          return py::array(dtype, total, static_cast<RawData*>(data),
                           py::capsule(data, [](void* p) { std::free(p); }));
      },
      "将分时线记录转换为NumPy元组");

    m.def(
      "timeline_to_df",
      [](const TimeLineList& timeline) {
          size_t total = timeline.size();
          if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
          }

          // 创建数组
          py::array_t<int64_t> datetime_arr(total);
          py::array_t<double> price_arr(total);
          py::array_t<double> vol_arr(total);

          // 获取缓冲区并填充数据
          auto datetime_buf = datetime_arr.request();
          auto price_buf = price_arr.request();
          auto vol_buf = vol_arr.request();

          int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
          double* price_ptr = static_cast<double*>(price_buf.ptr);
          double* vol_ptr = static_cast<double*>(vol_buf.ptr);

          for (size_t i = 0; i < total; i++) {
              const TimeLineRecord& record = timeline[i];
              datetime_ptr[i] = record.datetime.timestamp() * 1000LL;
              price_ptr[i] = record.price;
              vol_ptr[i] = record.vol;
          }

          // 构建 DataFrame
          py::dict columns;
          columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
          columns["price"] = price_arr;
          columns["vol"] = vol_arr;

          return py::module_::import("pandas").attr("DataFrame")(columns, py::arg("copy") = false);
      },
      "将分时线记录转换为 DataFrame");
}
