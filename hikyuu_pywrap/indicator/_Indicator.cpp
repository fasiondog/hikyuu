/*
 * _Indicator.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <hikyuu/indicator/Indicator.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

string (Indicator::*ind_read_name)() const = &Indicator::name;
void (Indicator::*ind_write_name)(const string&) = &Indicator::name;

void (Indicator::*setContext_1)(const Stock&, const KQuery&) = &Indicator::setContext;
void (Indicator::*setContext_2)(const KData&) = &Indicator::setContext;

Indicator (Indicator::*ind_call_1)(const Indicator&) = &Indicator::operator();
Indicator (Indicator::*ind_call_2)(const KData&) = &Indicator::operator();
Indicator (Indicator::*ind_call_3)() = &Indicator::operator();

void (Indicator::*setIndParam1)(const string&, const Indicator&) = &Indicator::setIndParam;
void (Indicator::*setIndParam2)(const string&, const IndParam&) = &Indicator::setIndParam;

void export_Indicator(py::module& m) {
    py::class_<Indicator>(m, "Indicator", "技术指标")
      .def(py::init<>())
      .def(py::init<IndicatorImpPtr>(), py::keep_alive<1, 2>())
      .def("__str__", to_py_str<Indicator>)
      .def("__repr__", to_py_str<Indicator>)

      .def_property("name", ind_read_name, ind_write_name, "指标名称")
      .def_property_readonly("long_name", &Indicator::long_name,
                             "返回形如：Name(param1_val,param2_val,...)")
      .def_property_readonly("discard", &Indicator::discard, "结果中需抛弃的个数")

      .def("set_discard", &Indicator::setDiscard, R"(set_discard(self, discard)
    
    设置抛弃的个数，如果小于原有的discard则无效
    :param int discard: 需抛弃的点数，大于0)")

      .def("get_param", &Indicator::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &Indicator::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string | Query | KData | Stock | DatetimeList
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &Indicator::haveParam, "是否存在指定参数")

      .def("support_ind_param", &Indicator::supportIndParam, "是否支持动态指标参数")
      .def("have_ind_param", &Indicator::haveIndParam, "是否存在指定的动态指标参数")
      .def("get_ind_param", &Indicator::getIndParam, R"(get_ind_param(self, name)
    
    获取指定的动态指标参数
    
    :param str name: 参数名称
    :return: 动态指标参数
    :rtype: IndParam
    :raises out_of_range: 无此参数)")

      .def("set_ind_param", setIndParam1)
      .def("set_ind_param", setIndParam2, R"(set_param(self, name, ind)

    设置动态指标参数

    :param str name: 参数名称
    :param Indicator|IndParam: 参数值（可为 Indicator 或 IndParam 实例）)")

      .def("empty", &Indicator::empty, "是否为空")
      .def("clone", &Indicator::clone, "克隆操作")
      .def("formula", &Indicator::formula, R"(formula(self)

    打印指标公式

    :rtype: str)")

      .def("get_result_num", &Indicator::getResultNumber, R"(get_result_num(self)

    获取结果集数量

    :rtype: int)")

      .def("get", &Indicator::get, py::arg("pos"), py::arg("result_index") = 0,
           R"(get(self, pos[, result_index=0])

    获取指定位置的值

    :param int pos: 指定的位置索引
    :param int result_index: 指定的结果集
    :rtype: float)")

      .def("get_pos", &Indicator::getPos, R"(get_pos(self, date):

    获取指定日期相应的索引位置

    :param Datetime date: 指定日期
    :rtype: int)")

      .def("get_datetime", &Indicator::getDatetime, R"(get_datetime(self, pos)

    获取指定位置的日期

    :param int pos: 指定的位置索引
    :rtype: float)")

      .def("get_by_datetime", &Indicator::getByDate, py::arg("datetime"),
           py::arg("result_index") = 0,
           R"(get_by_datetime(self, date[, result_index=0])

    获取指定日期数值。如果对应日期无结果，返回 constant.null_price

    :param Datetime datetime: 指定日期
    :param int result_index: 指定的结果集
    :rtype: float)")

      .def("get_result", &Indicator::getResult, R"(get_result(self, result_index)

    获取指定结果集

    :param int result_index: 指定的结果集
    :rtype: Indicator)")

      .def("get_result_as_price_list", &Indicator::getResultAsPriceList,
           R"(get_result_as_price_list(self, result_index)

    获取指定结果集

    :param int result_index: 指定的结果集
    :rtype: PriceList)")

      .def("get_datetime_list", &Indicator::getDatetimeList, R"(get_datetime_list(self)

    返回对应的日期列表

    :rtype: DatetimeList)")

      .def("set_context", setContext_1)
      .def("set_context", setContext_2, R"(set_context(self, kdata)

    设置上下文

    :param KData kdata: 关联的上下文K线)
      
set_context(self, stock, query)

    设置上下文

    :param Stock stock: 指定的 Stock
    :param Query query: 指定的查询条件)")

      .def("get_context", &Indicator::getContext, R"(get_context(self)

    获取上下文

    :rtype: KData)")

      .def("equal", &Indicator::equal)
      .def("is_same", &Indicator::isSame)
      .def("get_imp", &Indicator::getImp)
      .def("__len__", &Indicator::size)

      .def("__call__", ind_call_1)
      .def("__call__", ind_call_2)
      .def("__call__", ind_call_3)

      .def(
        "to_np",
        [](const Indicator& self) {
            py::array_t<Indicator::value_t> ret;
            auto imp = self.getImp();
            HKU_IF_RETURN(!imp, ret);
            ret = py::array_t<Indicator::value_t>(self.size(), imp->data(0));
            return ret;
        },
        "转化为np.array，如果indicator存在多个值，只返回第一个")

      .def(py::self + py::self)
      .def(py::self + Indicator::value_t())
      .def(Indicator::value_t() + py::self)

      .def(py::self - py::self)
      .def(py::self - Indicator::value_t())
      .def(Indicator::value_t() - py::self)

      .def(py::self * py::self)
      .def(py::self * Indicator::value_t())
      .def(Indicator::value_t() * py::self)

      .def(py::self / py::self)
      .def(py::self / Indicator::value_t())
      .def(Indicator::value_t() / py::self)

      .def(py::self == py::self)
      .def(py::self == Indicator::value_t())
      .def(Indicator::value_t() == py::self)

      .def(py::self != py::self)
      .def(py::self != Indicator::value_t())
      .def(Indicator::value_t() != py::self)

      .def(py::self >= py::self)
      .def(py::self >= Indicator::value_t())
      .def(Indicator::value_t() >= py::self)

      .def(py::self <= py::self)
      .def(py::self <= Indicator::value_t())
      .def(Indicator::value_t() <= py::self)

      .def(py::self > py::self)
      .def(py::self > Indicator::value_t())
      .def(Indicator::value_t() > py::self)

      .def(py::self < py::self)
      .def(py::self < Indicator::value_t())
      .def(Indicator::value_t() < py::self)

      .def(py::self % py::self)
      .def(py::self % Indicator::value_t())
      .def(Indicator::value_t() % py::self)

      .def(py::self & py::self)
      .def(py::self & Indicator::value_t())
      .def(Indicator::value_t() & py::self)

      .def(py::self | py::self)
      .def(py::self | Indicator::value_t())
      .def(Indicator::value_t() | py::self)

        DEF_PICKLE(Indicator);
}
