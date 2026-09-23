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
    py::class_<TimeLineRecord>(m, "TimeLineRecord", "The time-line record; the attributes are readable and writable")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t, price_t>())
      .def("__str__", to_py_str<TimeLineRecord>)
      .def("__repr__", to_py_str<TimeLineRecord>)
      .def_readwrite("date", &TimeLineRecord::datetime, "The date-time")
      .def_readwrite("price", &TimeLineRecord::price, "The price")
      .def_readwrite("vol", &TimeLineRecord::vol, "The volume")
      .def(py::self == py::self)

        DEF_PICKLE(TimeLineRecord);

    m.def(
      "timeline_to_np",
      [](const TimeLineList& timeline) {
          size_t total = timeline.size();
          HKU_IF_RETURN(total == 0, py::array());

          struct RawData {
              int64_t datetime;  // The converted millisecond timestamp
              double price;
              double vol;
          };

          // Allocate the memory with malloc
          RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
          for (size_t i = 0, len = timeline.size(); i < len; i++) {
              const TimeLineRecord& record = timeline[i];
              data[i].datetime = record.datetime.timestamp() * 1000LL;
              data[i].price = record.price;
              data[i].vol = record.vol;
          }

          // Define the NumPy structured data type
          auto dtype = py::dtype(vector_to_python_list<string>({"datetime", "price", "vol"}),
                                 vector_to_python_list<string>({"datetime64[ns]", "d", "d"}),
                                 vector_to_python_list<int64_t>({0, 8, 16}), 24);

          // Manage the memory with the capsule
          return py::array(dtype, total, static_cast<RawData*>(data),
                           py::capsule(data, [](void* p) { std::free(p); }));
      },
      "Convert the time-line records to a NumPy tuple");

    m.def(
      "timeline_to_df",
      [](const TimeLineList& timeline) {
          size_t total = timeline.size();
          if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
          }

          // Create the array
          py::array_t<int64_t> datetime_arr(total);
          py::array_t<double> price_arr(total);
          py::array_t<double> vol_arr(total);

          // Get the buffer and fill the data
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

          // Build the DataFrame
          py::dict columns;
          columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
          columns["price"] = price_arr;
          columns["vol"] = vol_arr;

          return py::module_::import("pandas").attr("DataFrame")(columns, py::arg("copy") = false);
      },
      "Convert the time-line records to a DataFrame");
}
