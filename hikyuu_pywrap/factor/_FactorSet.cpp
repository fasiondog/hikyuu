/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-24
 *      Author: fasiondog
 */

#include <hikyuu/factor/FactorSet.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_FactorSet(py::module& m) {
    py::class_<FactorSet>(m, "FactorSet", "因子元数据")
      .def(py::init<>(), R"(__init__(self)
    
    默认构造函数，创建空的因子集)")

      .def(py::init<const string&, const KQuery::KType&, const Block&>(), py::arg("name"),
           py::arg("ktype") = KQuery::DAY, py::arg("block") = Block(),
           R"(__init__(self, name[, ktype=KQuery.DAY[, block=Block()]])
    
    构造函数，创建指定名称和类型的因子集

    :param str name: 因子集名称
    :param KQuery.KType ktype: K线类型，默认为日线
    :param Block block: 板块信息，证券集合，默认为空)")

      .def(py::init<const IndicatorList&, const KQuery::KType&>(), py::arg("inds"),
           py::arg("ktype") = KQuery::DAY,
           R"(__init__(self, inds[, ktype=KQuery.DAY])
    
    构造函数，使用指定的指标列表创建因子集合，因子名称默认为指标名称

    :note: 同名的指标会被覆盖，最终保留最后一个同名指标
    :param list inds: 指标列表
    :param KQuery.KType ktype: 因子集合的K线类型，默认为日线)")

      .def("__str__", &FactorSet::str)
      .def("__repr__", &FactorSet::str)

      .def_property("name", py::overload_cast<>(&FactorSet::name, py::const_),
                    py::overload_cast<const string&>(&FactorSet::name),
                    py::return_value_policy::copy, "因子名称")
      .def_property("ktype", py::overload_cast<>(&FactorSet::ktype, py::const_),
                    py::overload_cast<const string&>(&FactorSet::ktype),
                    py::return_value_policy::copy, "因子频率类型")

      .def_property("block", py::overload_cast<>(&FactorSet::block, py::const_),
                    py::overload_cast<const Block&>(&FactorSet::block),
                    py::return_value_policy::copy, "因子集合对应板块")

      .def("is_null", &FactorSet::isNull, R"(is_null(self)
        
    是否为null值)")

      .def("empty", &FactorSet::empty, R"(empty(self)
    
    是否为空)")

      .def("clear", &FactorSet::clear, R"(clear(self))

    清空因子元数据)")

      .def("have", &FactorSet::have)
      .def("remove", &FactorSet::remove)
      .def("add", py::overload_cast<const Factor&>(&FactorSet::add))
      .def("add", py::overload_cast<const Indicator&>(&FactorSet::add))
      .def("add", py::overload_cast<const IndicatorList&>(&FactorSet::add))
      .def("add", py::overload_cast<const FactorList&>(&FactorSet::add))

      .def("get_factors", &FactorSet::getAllFactors, py::return_value_policy::copy, "获取因子列表")

      .def(
        "get_all_values", &FactorSet::getAllValues, py::arg("query"), py::arg("align") = false,
        py::arg("fill_null") = false, py::arg("tovalue") = true,
        py::arg("align_dates") = DatetimeList{},
        R"(get_all_values(self, query[, align=False[, fill_null=False[, tovalue=True[, align_dates=DatetimeList()]]]])
    
    获取所有因子的指定查询参数的计算结果

    :param Query query: 查询参数
    :param bool align: 是否进行日期对齐(如按指定align_dates或默认交易日历)，默认 False
    :param bool fill_null: 是否填充空值，默认 False
    :param bool tovalue: 是否转换为数值，默认 True
    :param DatetimeList align_dates: 对齐日期列表，默认为空
    :return: 所有因子的计算结果列表
    :rtype: list)")

      .def(
        "get_values",
        [](FactorSet& self, const py::object& stks, const KQuery& query, bool align = false,
           bool fill_null = false, bool tovalue = true, bool check = false,
           const DatetimeList& align_dates = DatetimeList{}) {
            return self.getValues(get_stock_list_from_python(stks), query, align, fill_null,
                                  tovalue, check, align_dates);
        },
        py::arg("stocks"), py::arg("query"), py::arg("align") = false, py::arg("fill_null") = false,
        py::arg("tovalue") = false, py::arg("check") = false,
        py::arg("align_dates") = DatetimeList{},
        R"(get_values(self, stocks, query[, align=False[, fill_null=False[, tovalue=False[, check=False[, align_dates=DatetimeList()]]]]])
    
    获取指定股票列表的指定查询参数的计算结果

    :param list stocks: 证券列表
    :param Query query: 查询参数
    :param bool align: 是否进行日期对齐(如按指定align_dates或默认交易日历)，默认 False
    :param bool fill_null: 是否填充空值，默认 False
    :param bool tovalue: 是否转换为数值，默认 False
    :param bool check: 是否检查股票列表属于自身指定的 block，默认 False
    :param DatetimeList align_dates: 对齐日期列表，默认为空
    :return: 按股票顺序排列的计算结果列表
    :rtype: list)")

      .def("save_to_db", &FactorSet::save_to_db,
           R"(save_to_db(self)
    
    保存因子集到数据库
    
    :note: 以 name + ktype 作为唯一标识)")

      .def("remove_from_db", &FactorSet::remove_from_db,
           R"(remove_from_db(self)
    
    从数据库中删除因子集
    
    :note: 以 name + ktype 作为唯一标识)")

      .def("load_from_db", &FactorSet::load_from_db,
           R"(load_from_db(self)
    
    从数据库中加载因子集
    
    :note: 以 name + ktype 作为唯一标识，如果不存在则不修改当前对象)")

      .def("__getitem__", py::overload_cast<const string&>(&FactorSet::get, py::const_),
           py::return_value_policy::copy)
      .def("__getitem__", py::overload_cast<size_t>(&FactorSet::get, py::const_),
           py::return_value_policy::copy)
      .def("__len__", &FactorSet::size, "包含的因子数量")
      .def(
        "__iter__",
        [](const FactorSet& self) { return py::make_iterator(self.begin(), self.end()); },
        py::keep_alive<0, 1>())

        DEF_PICKLE(FactorSet);
}