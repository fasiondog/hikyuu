/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <hikyuu/factor/Factor.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_Factor(py::module& m) {
    py::class_<Factor>(m, "Factor", "因子元数据")
      .def(py::init<>(), R"(__init__(self)
    
    默认构造函数，创建空的因子对象)")

      .def(py::init<const string&, const KQuery::KType&>(), py::arg("name"),
           py::arg("ktype") = KQuery::DAY,
           R"(__init__(self, name[, ktype=KQuery.DAY])
    
    构造函数，只指定因子名称和K线类型，将尝试自动从数据库加载因子

    :param str name: 因子名称
    :param KQuery.KType ktype: K线类型，默认为日线)")

      .def(
        py::init<const string&, const Indicator&, const KQuery::KType&, const string&,
                 const string&, bool, const Datetime&, const Block&>(),
        py::arg("name"), py::arg("formula"), py::arg("ktype") = KQuery::DAY, py::arg("brief") = "",
        py::arg("details") = "", py::arg("need_save_value") = false,
        py::arg("start_date") = Datetime::min(), py::arg("block") = Block(),
        R"(__init__(self, name, formula[, ktype=KQuery.DAY[, brief=""[, details=""[, need_save_value=False[, start_date=Datetime.min()[, block=Block()]]]]]])
    
    构造函数，创建新的因子对象（因子名称 + K线类型 为因子的唯一标识）

    :param str name: 因子名称
    :param Indicator formula: 计算公式指标，一旦创建不可更改
    :param KQuery.KType ktype: K线类型，默认为日线
    :param str brief: 简要描述，默认为空
    :param str details: 详细描述，默认为空
    :param bool need_save_value: 是否需要持久化保存因子值数据，默认为False
    :param Datetime start_date: 开始日期，数据存储时的起始日期，默认为最小日期
    :param Block block: 板块信息，证券集合，如果为空则为全部，默认为空
    :note: 因子名称不区分大小写，以 name + ktype 作为唯一标识)")

      .def("__str__", &Factor::str)
      .def("__repr__", &Factor::str)

      .def_property("name", py::overload_cast<>(&Factor::name, py::const_),
                    py::overload_cast<const string&>(&Factor::name), py::return_value_policy::copy,
                    "因子名称")
      .def_property("ktype", py::overload_cast<>(&Factor::ktype, py::const_),
                    py::overload_cast<const string&>(&Factor::ktype), py::return_value_policy::copy,
                    "因子频率类型")
      .def_property("create_at", py::overload_cast<>(&Factor::createAt, py::const_),
                    py::overload_cast<const Datetime&>(&Factor::createAt),
                    py::return_value_policy::copy, "创建日期")
      .def_property("update_at", py::overload_cast<>(&Factor::updateAt, py::const_),
                    py::overload_cast<const Datetime&>(&Factor::updateAt),
                    py::return_value_policy::copy, "更改日期")
      .def_property("formula", py::overload_cast<>(&Factor::formula, py::const_),
                    py::overload_cast<const Indicator&>(&Factor::formula),
                    py::return_value_policy::copy, "因子公式")
      .def_property("start_date", py::overload_cast<>(&Factor::startDate, py::const_),
                    py::overload_cast<const Datetime&>(&Factor::startDate), "数据存储起始日期")
      .def_property("block", py::overload_cast<>(&Factor::block, py::const_),
                    py::overload_cast<const Block&>(&Factor::block), py::return_value_policy::copy,
                    "证券集合")
      .def_property("brief", py::overload_cast<>(&Factor::brief, py::const_),
                    py::overload_cast<const string&>(&Factor::brief), py::return_value_policy::copy,
                    "基础说明")
      .def_property("details", py::overload_cast<>(&Factor::details, py::const_),
                    py::overload_cast<const string&>(&Factor::details),
                    py::return_value_policy::copy, "详细说明")
      .def_property("need_save_value", py::overload_cast<>(&Factor::needSaveValue, py::const_),
                    py::overload_cast<bool>(&Factor::needSaveValue), "是否持久化保存因子值数据")

      .def("save_to_db", &Factor::save_to_db,
           R"(save_to_db(self)
    
    保存因子元数据到数据库，如果因子已存在则更新，否则插入新记录
    
    :note: 因子名称不区分大小写，以 name + ktype 作为唯一标识)")

      .def(
        "save_special_values_to_db",
        py::overload_cast<const Stock&, const Indicator&, bool>(&Factor::save_special_values_to_db),
        py::arg("stock"), py::arg("values"), py::arg("replace") = false,
        R"(save_special_values_to_db(self, stock, values[, replace=False])
    
    特殊因子保存值到数据库, 支持两种输入格式：
    1. 直接保存Indicator对象的结果数据
    2. 保存预计算的日期-值对数据
    
    重载版本1 - 保存Indicator对象, 通常为PRICELIST:
    :param Stock stock: 证券对象
    :param Indicator values: 已计算好的指标对象（必须已绑定K线数据）
    :param bool replace: 是否替换已有数据，默认False
    
    重载版本2 - 保存预计算数据:
    :param Stock stock: 证券对象  
    :param DatetimeList dates: 特殊因子日期列表
    :param PriceList values: 特殊因子值列表
    :param bool replace: 是否替换已有数据，默认False
    
    使用场景:
    - 保存复合指标计算结果
    - 保存外部导入的财务数据
    - 保存机器学习模型预测结果
    - 保存人工标注的特殊因子值)")

      .def("remove_from_db", &Factor::remove_from_db,
           R"(remove_from_db(self)
    
    从数据库中删除因子及其数据。注：为防止误操作，特殊因子的值不会删除，需自行手工删除。
    
    :note: 以 name + ktype 作为唯一标识进行删除)")

      .def("load_from_db", &Factor::load_from_db,
           R"(load_from_db(self)
    
    从数据库中加载因子元数据
    
    :note: 以 name + ktype 作为唯一标识进行加载)")

      .def(
        "get_all_values", &Factor::getAllValues, py::arg("query"), py::arg("align") = false,
        py::arg("fill_null") = false, py::arg("tovalue") = false,
        py::arg("align_dates") = DatetimeList{},
        R"(get_all_values(self, query[, align=False[, fill_null=False[, tovalue=False[, align_dates=DatetimeList()]]]])
    
    获取指定查询参数的所有计算结果

    :param Query query: 查询参数
    :param bool align: 是否进行日期对齐(如按指定align_dates或默认交易日历)，默认 False
    :param bool fill_null: 是否填充空值，默认 False
    :param bool tovalue: 是否转换为数值，默认 False
    :param DatetimeList align_dates: 对齐日期列表，默认为空
    :return: 所有股票的计算结果列表
    :rtype: list)")

      .def(
        "get_values",
        [](Factor& self, const py::object& stks, const KQuery& query, bool align, bool fill_null,
           bool tovalue, bool check, const DatetimeList& align_dates) {
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

      .def(py::hash(py::self))

        DEF_PICKLE(Factor);
}
