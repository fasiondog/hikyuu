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

void export_TransRecord(py::module& m) {
    py::class_<TransRecord>(m, "TransRecord")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t, price_t, int>())
      .def("__str__", to_py_str<TransRecord>)
      .def("__repr__", to_py_str<TransRecord>)
      .def_readwrite("date", &TransRecord::datetime, "时间")
      .def_readwrite("price", &TransRecord::price, "价格")
      .def_readwrite("vol", &TransRecord::vol, "成交量")
      .def_readwrite("direct", &TransRecord::direct,
                     "买卖盘性质: 1--sell 0--buy 2--集合竞价 其他未知")
      .def(py::self == py::self)

        DEF_PICKLE(TransRecord);

    m.def(
      "translist_to_np",
      [](const TransList& trans) {
          size_t total = trans.size();
          HKU_IF_RETURN(total == 0, py::array());

          struct RawData {
              int64_t datetime;
              double price;
              double vol;
              int64_t direct;
          };

          // 使用 malloc 分配内存
          RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
          for (size_t i = 0, len = trans.size(); i < len; i++) {
              const TransRecord& record = trans[i];
              data[i].datetime = record.datetime.timestamp() * 1000LL;
              data[i].price = record.price;
              data[i].vol = record.vol;
              data[i].direct = record.direct;
          }

          // 定义NumPy结构化数据类型
          auto dtype =
            py::dtype(vector_to_python_list<string>({"datetime", "price", "vol", "direct"}),
                      vector_to_python_list<string>({"datetime64[ns]", "d", "d", "i4"}),
                      vector_to_python_list<int64_t>({0, 8, 16, 24}), 32);

          // 使用 capsule 管理内存
          return py::array(dtype, total, static_cast<RawData*>(data),
                           py::capsule(data, [](void* p) { std::free(p); }));
      },
      "将分笔记录转换为NumPy元组");

    m.def(
      "translist_to_df",
      [](const TransList& trans) {
          size_t total = trans.size();
          if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
          }

          // 创建数组
          py::array_t<int64_t> datetime_arr(total);
          py::array_t<double> price_arr(total);
          py::array_t<double> vol_arr(total);
          py::array_t<int64_t> direct_arr(total);

          // 获取缓冲区并填充数据
          auto datetime_buf = datetime_arr.request();
          auto price_buf = price_arr.request();
          auto vol_buf = vol_arr.request();
          auto direct_buf = direct_arr.request();

          int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
          double* price_ptr = static_cast<double*>(price_buf.ptr);
          double* vol_ptr = static_cast<double*>(vol_buf.ptr);
          int64_t* direct_ptr = static_cast<int64_t*>(direct_buf.ptr);

          for (size_t i = 0; i < total; i++) {
              const TransRecord& record = trans[i];
              datetime_ptr[i] = record.datetime.timestamp() * 1000LL;
              price_ptr[i] = record.price;
              vol_ptr[i] = record.vol;
              direct_ptr[i] = record.direct;
          }

          // 构建 DataFrame
          py::dict columns;
          columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
          columns["price"] = price_arr;
          columns["vol"] = vol_arr;
          columns["direct"] = direct_arr;
          return py::module_::import("pandas").attr("DataFrame")(columns, py::arg("copy") = false);
      },
      "将分笔记录转换为 DataFrame");
}
