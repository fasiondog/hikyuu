/*
 * _KDataDriver.cpp
 *
 *  Created on: 2017年10月7日
 *      Author: fasiondog
 */

#include "_KDataDriver.h"

using namespace hku;
namespace py = pybind11;

void export_KDataDriver(py::module& m) {
    py::class_<KDataDriver, KDataDriverPtr, PyKDataDriver>(m, "KDataDriver",
                                                           R"(K线数据驱动基类

  子类接口:
    - _init(self)
    - isIndexFirst(self) (必须)
    - canParallelLoad(self) (必须)
    - getCount(self, market, code, ktype)
    - _getIndexRangeByDate(self, market, code, query)
    - _getKRecordList(self, market, code, query)
    - _getTimeLineList(self, market, code, query)
    - _getTransList(self, market, code, query)
  )")
      .def(py::init<>())
      .def(py::init<const string&>())
      .def_property_readonly("name", &KDataDriver::name, py::return_value_policy::copy, "驱动名称")

      .def("__str__", to_py_str<KDataDriver>)
      .def("__repr__", to_py_str<KDataDriver>)

      .def("get_param", &KDataDriver::getParam<boost::any>, "获取指定参数的值")
      .def("set_param",
           static_cast<void (KDataDriver::*)(const std::string&, const boost::any&)>(
             &KDataDriver::setParam),
           "设置参数")
      .def("have_param", &KDataDriver::haveParam, "指定参数是否存在")

      .def("clone", &KDataDriver::clone, "克隆驱动")

      .def("_init", &KDataDriver::_init, "【子类接口】初始化驱动")
      .def("isIndexFirst", &KDataDriver::isIndexFirst,
           "【子类接口（必须）】判断该引擎是否是位置索引方式查询速度更快，还是按日期方式查询更快")
      .def("canParallelLoad", &KDataDriver::canParallelLoad,
           "【子类接口（必须）】是否支持并行数据加载")
      .def("getCount", &KDataDriver::getCount, py::arg("market"), py::arg("code"), py::arg("ktype"),
           R"(获取指定类型的K线数据量

    :param str market: 市场简称
    :param str code: 证券代码
    :param Query.KType ktype: K线类型
    :rtype int)")
      .def(
        "_getIndexRangeByDate",
        [](KDataDriver& self, const string& market, const string& code, const KQuery& query) {
            size_t start, end;
            self.getIndexRangeByDate(market, code, query, start, end);
            return py::make_tuple(start, end);
        },
        py::arg("market"), py::arg("code"), py::arg("query"),
        R"(【子类接口】获取指定日期范围对应的K线记录索引

    :param str market: 市场简称
    :param str code: 证券代码
    :param KQuery query: 查询条件
    :return: (start, end) 对应的K线记录位置)")
      .def("_getKRecordList", &KDataDriver::getKRecordList, py::arg("market"), py::arg("code"),
           py::arg("query"), "【子类接口】获取K线数据")
      .def("_getTimeLineList", &KDataDriver::getTimeLineList, py::arg("market"), py::arg("code"),
           py::arg("query"), "【子类接口】获取分时线数据")
      .def("_getTransList", &KDataDriver::getTransList, py::arg("market"), py::arg("code"),
           py::arg("query"), "【子类接口】获取历史分笔数据")
      .def("isColumnFirst", &KDataDriver::isColumnFirst, "是否列优先(列数据库存储K线数据)");
}