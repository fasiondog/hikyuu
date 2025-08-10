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
          struct RawData {
              int64_t datetime;
              double price;
              double vol;
              int64_t direct;
          };

          std::vector<RawData> data;
          data.resize(trans.size());
          for (size_t i = 0; i < trans.size(); i++) {
              const TransRecord& record = trans[i];
              data[i].datetime = record.datetime.timestamp() * 1000LL;
              data[i].price = record.price;
              data[i].vol = record.vol;
              data[i].direct = record.direct;
          }

          // 定义NumPy结构化数据类型
          auto dtype =
            py::dtype(vector_to_python_list<string>({"datetime", "price", "vol", "direct"}),
                      vector_to_python_list<string>({"datetime64[ns]", "d", "d", "i8"}),
                      vector_to_python_list<int64_t>({0, 8, 16, 24}), 32);

          return py::array(dtype, data.size(), data.data());
      },
      "将分笔记录转换为NumPy元组");

    m.def(
      "translist_to_df",
      [](const TransList& trans) {
          size_t total = trans.size();
          if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
          }

          std::vector<int64_t> datetime(total), direct(total);
          std::vector<double> price(total), vol(total);

          for (size_t i = 0; i < trans.size(); i++) {
              const TransRecord& record = trans[i];
              datetime[i] = record.datetime.timestamp() * 1000LL;
              price[i] = record.price;
              vol[i] = record.vol;
              direct[i] = record.direct;
          }

          py::dict columns;
          columns["datetime"] =
            py::array_t<int64_t>(total, datetime.data()).attr("astype")("datetime64[ns]");
          columns["price"] = py::array_t<price_t>(total, price.data(), py::dtype("float64"));
          columns["vol"] = py::array_t<price_t>(total, vol.data(), py::dtype("float64"));
          columns["direct"] = py::array_t<int64_t>(total, direct.data(), py::dtype("i8"));
          return py::module_::import("pandas").attr("DataFrame")(columns);
      },
      "将分笔记录转换为 DataFrame");
}
