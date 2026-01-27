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
      .def("__str__", &Indicator::str)
      .def("__repr__", &Indicator::str)

      .def_property_static(
        "enable_increment_calculate",
        [](py::object) { return Indicator::enableIncrementCalculate(); },
        [](py::object cls, bool flag) { Indicator::enableIncrementCalculate(flag); },
        "启用/禁用指标增量计算")

      .def_property("name", ind_read_name, ind_write_name, "指标名称")
      .def_property_readonly("long_name", &Indicator::long_name,
                             "返回形如：Name(param1_val,param2_val,...)")
      .def_property_readonly("discard", &Indicator::discard, "结果中需抛弃的个数")
      .def_property_readonly("optype",
                             [](const Indicator& ind) { return getOPTypeName(ind.getOPType()); })

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

      .def("have_ind_param", &Indicator::haveIndParam, "是否存在指定的动态周期指标参数")
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

      .def(
        "get_pos",
        [](const Indicator& self, const Datetime& d) {
            size_t pos = self.getPos(d);
            py::object ret = py::none();
            if (pos != Null<size_t>()) {
                ret = py::int_(pos);
            }
            return ret;
        },
        R"(get_pos(self, date):

    获取指定日期相应的索引位置, 如果没有对应位置返回 None

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

      .def("exist_nan", &Indicator::existNan, py::arg("result_idx=0"),
           R"(exist_nan(self, result_index)

    判断是否存在NaN值

    :param int result_index: 指定的结果集
    :rtype: bool)")

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

      .def("extend", &Indicator::extend, R"(extend(self)

    在有上下文时，自动将上下文扩展至当前最新数据并计算)")

      .def("contains", &Indicator::contains, R"(contains(self, name)
        
    获取指标公式中是否包含指定名称的指标
    
    :param str name: 指定的指标名称
    :rtype: bool)")

      .def("equal", &Indicator::equal)
      .def("is_same", &Indicator::isSame)
      .def("get_imp", &Indicator::getImp)
      .def("__len__", &Indicator::size)

      .def("__call__", ind_call_1)
      .def("__call__", ind_call_2)
      .def("__call__", ind_call_3)

      .def("__hash__", [](const Indicator& self) { return std::hash<Indicator>()(self); })

      .def("__getitem__",
           [](const Indicator& self, py::object obj) {
               py::object ret;
               if (py::isinstance<py::int_>(obj)) {
                   int64_t i = obj.cast<int64_t>();
                   int64_t length = self.size();
                   int64_t index = i < 0 ? length + i : i;
                   if (index < 0 || index >= length)
                       throw std::out_of_range(fmt::format("index out of range: {}", i));
                   ret = py::cast(self[index]);
                   return ret;
               } else if (py::isinstance<Datetime>(obj)) {
                   Datetime dt = py::cast<Datetime>(obj);
                   auto val = self[dt];
                   if (val == Null<Indicator::value_t>()) {
                       throw std::out_of_range(fmt::format("datetime out of range: {}", dt));
                   }
                   ret = py::cast(val);
                   return ret;
               } else if (py::isinstance<py::str>(obj)) {
                   Datetime dt = Datetime(py::cast<std::string>(obj));
                   auto val = self[dt];
                   if (val == Null<Indicator::value_t>()) {
                       throw std::out_of_range(fmt::format("datetime out of range: {}", dt));
                   }
                   ret = py::cast(val);
                   return ret;
               } else if (py::isinstance<py::slice>(obj)) {
                   py::slice slice = py::cast<py::slice>(obj);
                   size_t start, stop, step, length;

                   if (!slice.compute(self.size(), &start, &stop, &step, &length)) {
                       throw std::invalid_argument("无效的切片参数");
                   }

                   std::vector<Indicator::value_t> result;
                   result.reserve(length);
                   for (size_t i = 0; i < length; ++i) {
                       size_t index = start + i * step;
                       result.push_back(self[static_cast<size_t>(index)]);
                   }

                   ret = py::cast(result);
                   return ret;
               }

               throw std::out_of_range("Error index type");
           })

      .def(
        "__iter__",
        [](const Indicator& self) { return py::make_iterator(self.begin(), self.end()); },
        py::keep_alive<0, 1>())

      .def(
        "to_np",
        [](const Indicator& self) {
            py::array ret;
            auto imp = self.getImp();
            HKU_IF_RETURN(!imp, ret);
            size_t ret_num = imp->getResultNumber();

            uint64_t* buffer = new uint64_t[self.size() * (ret_num + 1)];

            std::vector<string> names;
            std::vector<string> fields;
            std::vector<int64_t> offsets;

            auto dates = imp->getDatetimeList();
            size_t bytes_size;
            if (!dates.empty()) {
                names.push_back("datetime");
                fields.push_back("datetime64[ns]");
                offsets.push_back(0);
                for (size_t i = 0; i < ret_num; i++) {
                    names.push_back(fmt::format("value{}", i));
                    fields.push_back("d");
                    offsets.push_back(offsets.back() + sizeof(Indicator::value_t));
                }
                bytes_size = sizeof(Datetime) + ret_num * sizeof(Indicator::value_t);
            } else {
                for (size_t i = 0; i < ret_num; i++) {
                    names.push_back(fmt::format("value{}", i));
                    fields.push_back("d");
                    if (i == 0) {
                        offsets.push_back(0);
                    } else {
                        offsets.push_back(offsets.back() + sizeof(Indicator::value_t));
                    }
                }
                bytes_size = ret_num * sizeof(Indicator::value_t);
            }

            auto dtype =
              py::dtype(vector_to_python_list<string>(names), vector_to_python_list<string>(fields),
                        vector_to_python_list<int64_t>(offsets), bytes_size);

            std::vector<const Indicator::value_t*> src(ret_num);
            for (size_t i = 0; i < ret_num; i++) {
                src[i] = imp->data(i);
            }

            uint64_t* data = buffer;
            double* val = (double*)buffer;
            if (!dates.empty()) {
                size_t x = ret_num + 1;
                for (size_t i = 0, total = imp->size(); i < total; i++) {
                    data[i * x] = dates[i].timestamp() * 1000LL;
                    for (size_t j = 0; j < ret_num; j++) {
                        val[i * x + j + 1] = src[j][i];
                    }
                }
            } else {
                for (size_t i = 0, total = imp->size(); i < total; i++) {
                    for (size_t j = 0; j < ret_num; j++) {
                        val[i * ret_num + j] = src[j][i];
                    }
                }
            }

            auto capsule =
              py::capsule(buffer, [](void* ptr) { delete[] static_cast<uint64_t*>(ptr); });
            ret = py::array(dtype, self.size(), data, capsule);
            return ret;
        },
        "转化为np.array, 如果为时间序列, 则包含 datetime 日期列")

      .def(
        "value_to_np",
        [](const Indicator& self) {
            size_t ret_num = self.getResultNumber();

            // 初始化array_t并获取其内部缓冲区
            py::array_t<double> ret;
            ret.resize({self.size(), ret_num});  // 二维形状: [size, ret_num]
            auto buf = ret.request();
            double* buffer = static_cast<double*>(buf.ptr);  // 从array_t获取指针

            std::vector<std::string> names;
            std::vector<std::string> fields;
            std::vector<int64_t> offsets;
            for (size_t i = 0; i < ret_num; i++) {
                names.push_back(fmt::format("value{}", i));
                fields.push_back("d");
                offsets.push_back(i * sizeof(Indicator::value_t));  // 简化偏移计算
            }

            auto dtype = py::dtype(
              vector_to_python_list<std::string>(names), vector_to_python_list<std::string>(fields),
              vector_to_python_list<int64_t>(offsets), ret_num * sizeof(Indicator::value_t));

            std::vector<const Indicator::value_t*> src(ret_num);
            for (size_t i = 0; i < ret_num; i++) {
                src[i] = self.data(i);
            }

            // 填充数据到array_t的缓冲区
            for (size_t i = 0, total = self.size(); i < total; i++) {
                for (size_t j = 0; j < ret_num; j++) {
                    buffer[i * ret_num + j] = src[j][i];
                }
            }

            return py::array(dtype, {self.size()}, {ret_num * sizeof(double)}, buf.ptr, ret);
        },
        "仅转化值为np.array, 不包含日期列")

      .def(
        "to_array",
        [](const Indicator& self, size_t result_index) {
            HKU_CHECK(result_index < self.getResultNumber(), "result_index out of range");
            auto ret = py::array_t<double>(self.size());
            auto buf = ret.request();
            double* ptr = static_cast<double*>(buf.ptr);
            const auto* src = self.data(result_index);
            for (size_t i = 0; i < self.size(); i++) {
                ptr[i] = src[i];
            }
            return ret;
        },
        py::arg("result_index") = 0, "将指定结果集转化为numpy.array")

      .def(
        "to_df",
        [](const Indicator& self) {
            size_t total = self.size();
            if (total == 0) {
                return py::module_::import("pandas").attr("DataFrame")();
            }

            py::dict columns;
            auto dates = self.getDatetimeList();
            if (!dates.empty()) {
                std::vector<int64_t> datetime(total);
                for (size_t i = 0; i < total; i++) {
                    datetime[i] = dates[i].timestamp() * 1000LL;
                }
                columns["datetime"] =
                  py::array_t<int64_t>(total, datetime.data()).attr("astype")("datetime64[ns]");
            }

            size_t ret_num = self.getResultNumber();
            for (size_t i = 0; i < ret_num; i++) {
                py::array_t<double> arr(total);
                auto buf = arr.request();
                double* dst = static_cast<double*>(buf.ptr);
                const auto* src = self.data(i);
                for (size_t j = 0; j < total; j++) {
                    dst[j] = src[j];
                }
                columns[fmt::format("value{}", i).c_str()] = arr;
            }

            return py::module_::import("pandas").attr("DataFrame")(columns,
                                                                   py::arg("copy") = false);
        },
        "转换为 DataFrame")

      .def(
        "value_to_df",
        [](const Indicator& self) {
            size_t total = self.size();
            if (total == 0) {
                return py::module_::import("pandas").attr("DataFrame")();
            }

            py::dict columns;
            size_t ret_num = self.getResultNumber();
            for (size_t i = 0; i < ret_num; i++) {
                py::array_t<double> arr(total);
                auto buf = arr.request();
                double* dst = static_cast<double*>(buf.ptr);
                const auto* src = self.data(i);
                for (size_t j = 0; j < total; j++) {
                    dst[j] = src[j];
                }
                columns[fmt::format("value{}", i).c_str()] = arr;
            }

            return py::module_::import("pandas").attr("DataFrame")(columns,
                                                                   py::arg("copy") = false);
        },
        "转换为 DataFrame, 仅包含值")

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
