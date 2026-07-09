/*
 * _BaseInfoDriver.cpp
 *
 *  Created on: 2017年10月7日
 *      Author: fasiondog
 */

#include "_BaseInfoDriver.h"

using namespace hku;
namespace py = pybind11;

static string BaseInfoDriver_to_str(const BaseInfoDriver& v) {
    std::stringstream out;
    out << v;
    return out.str();
}

void export_BaseInfoDriver(py::module& m) {
    py::class_<BaseInfoDriver, BaseInfoDriverPtr, PyBaseInfoDriver>(m, "BaseInfoDriver",
                                                                    R"(基本信息数据获取驱动基类

    子类接口：
        - _init(self) (必须)
        - getAllStockInfo(self) (必须)
        - getStockInfo(self, market, code) (必须)
        - getMarketInfo(self, market) (必须)
        - getAllMarketInfo(self) (必须)
        - getAllStockTypeInfo(self) (必须)
        - getStockTypeInfo(self, type) (必须)
        - getAllHolidays(self) (必须)
        - getAllZhBond10(self) (必须)
        - getStockWeightList(self, market, code, start, end)
        - getAllStockWeightList(self)
        - getHistoryFinance(self, market, code, start, end)
        - getHistoryFinanceField(self)
        - getFinanceInfo(self, market, code)
    )")
      .def(py::init<const string&>(), R"(初始化

    :param str name: 驱动名称)")
      .def_property_readonly("name", &BaseInfoDriver::name, py::return_value_policy::copy,
                             "驱动名称")
      .def("__str__", BaseInfoDriver_to_str)
      .def("__repr__", BaseInfoDriver_to_str)

      .def("get_param", &BaseInfoDriver::getParam<boost::any>, "获取指定参数")
      .def("set_param",
           static_cast<void (BaseInfoDriver::*)(const std::string&, const boost::any&)>(
             &BaseInfoDriver::setParam),
           "设置指定参数")
      .def("have_param", &BaseInfoDriver::haveParam, "指定参数是否存在")

      .def("_init", &BaseInfoDriver::_init, "【子类接口（必须）】驱动初始化")
      .def("getAllStockInfo", &BaseInfoDriver::getAllStockInfo,
           "【子类接口（必须）】获取所有股票详情信息")
      .def("getStockInfo", &BaseInfoDriver::getStockInfo, py::arg("market"), py::arg("code"),
           R"(【子类接口（必须）】获取指定的证券信息

    :param str market: 市场简称
    :param str code: 证券代码)")
      .def("getStockWeightList", &BaseInfoDriver::getStockWeightList, py::arg("market"),
           py::arg("code"), py::arg("start"), py::arg("end"),
           R"(获取指定日期范围内 [start, end) 的权息列表

    :param str market: 市场简称
    :param str code: 证券代码
    :param Datetime start: 起始日期
    :param Datetime end: 结束日期)")
      .def("getAllStockWeightList", &BaseInfoDriver::getAllStockWeightList,
           "获取所有股票的权息列表")
      .def("getHistoryFinance", &BaseInfoDriver::getHistoryFinance, py::arg("market"),
           py::arg("code"), py::arg("start"), py::arg("end"),
           R"(获取历史财务信息

    :param str market: 市场简称
    :param str code: 证券代码
    :param Datetime start: 财务报告发布起始日期
    :param Datetime end: 查询结束日期)")
      .def("getHistoryFinanceField", &BaseInfoDriver::getHistoryFinanceField,
           "获取历史财务信息字段序号与名称")
      .def("getFinanceInfo", &BaseInfoDriver::getFinanceInfo, py::arg("market"), py::arg("code"),
           R"(获取当前财务信息

    :param str market: 市场标识
    :param str code: 证券代码)")
      .def("getMarketInfo", &BaseInfoDriver::getMarketInfo, py::arg("market"),
           R"(【子类接口（必须）】获取指定的MarketInfo

    :param str market: 市场简称
    :return: 如未找到，则返回 Null<MarketInfo>())")
      .def("getAllMarketInfo", &BaseInfoDriver::getAllMarketInfo,
           "【子类接口（必须）】获取全部市场信息")
      .def("getAllStockTypeInfo", &BaseInfoDriver::getAllStockTypeInfo,
           "【子类接口（必须）】获取全部证券类型信息")
      .def("getStockTypeInfo", &BaseInfoDriver::getStockTypeInfo, py::arg("type"),
           R"(【子类接口（必须）】获取相应的证券类型详细信息

    :param int type: 证券类型
    :return: 对应的证券类型信息，如果不存在，则返回Null<StockTypeInfo>())")
      .def("getAllHolidays", &BaseInfoDriver::getAllHolidays,
           "【子类接口（必须）】获取所有节假日日期")
      .def("getAllZhBond10", &BaseInfoDriver::getAllZhBond10,
           "【子类接口（必须）】获取所有中国10年期国债信息");
}