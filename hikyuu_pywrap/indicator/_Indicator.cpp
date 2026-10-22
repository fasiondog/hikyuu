/*
 * _Indicator.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <hikyuu/indicator/Indicator.h>
#include <cstring>
#include "../gil_policy.h"
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

// NumPy format char for the indicator value type: "f" (float32) under a low-precision build,
// otherwise "d" (float64); keeps the structured dtype consistent with Indicator::value_t.
template <typename T>
constexpr const char* indicatorValueNpField();
template <>
constexpr const char* indicatorValueNpField<double>() {
    return "d";
}
template <>
constexpr const char* indicatorValueNpField<float>() {
    return "f";
}

string (Indicator::*ind_read_name)() const = &Indicator::name;
void (Indicator::*ind_write_name)(const string&) = &Indicator::name;

void (Indicator::*setIndParam1)(const string&, const Indicator&) = &Indicator::setIndParam;
void (Indicator::*setIndParam2)(const string&, const IndParam&) = &Indicator::setIndParam;

void export_Indicator(py::module& m) {
    py::class_<Indicator>(m, "Indicator", "The technical indicator")
      .def(py::init<>())
      .def(py::init<IndicatorImpPtr>(), py::keep_alive<1, 2>())
      .def("__str__", &Indicator::str)
      .def("__repr__", &Indicator::str)

      .def_property_static(
        "enable_increment_calculate",
        [](py::object) { return Indicator::enableIncrementCalculate(); },
        [](py::object cls, bool flag) { Indicator::enableIncrementCalculate(flag); },
        "Enable/disable the indicator incremental calculation. NOTE: this is a process-global, "
        "special-purpose switch; set it once at startup (before any calculation) and do not "
        "toggle it at runtime")

      .def_property("name", ind_read_name, ind_write_name, "The indicator name")
      .def_property_readonly("long_name", &Indicator::long_name,
                             "Return in the form: Name(param1_val,param2_val,...)")
      .def_property_readonly("discard", &Indicator::discard,
                             "The number of the points to discard in the result")
      .def_property_readonly("optype",
                             [](const Indicator& ind) { return getOPTypeName(ind.getOPType()); })

      .def("set_discard", &Indicator::setDiscard, R"(set_discard(self, discard)
    
    Set the number to discard; the value is clamped to the indicator size. When the new value
    grows, the extended leading slots are filled with Null; when it shrinks, the previously
    discarded slots become part of the valid region.
    :param int discard: the number of the points to discard, greater than or equal to 0)")

      .def("get_param", &Indicator::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (Indicator::*)(const std::string&, const boost::any&)>(
             &Indicator::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :type value: int | bool | float | string | Query | KData | Stock | DatetimeList
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &Indicator::haveParam, "Whether the specified parameter exists")

      .def("have_ind_param", &Indicator::haveIndParam,
           "Whether the specified dynamic period indicator parameter exists")
      .def("get_ind_param", &Indicator::getIndParam, R"(get_ind_param(self, name)
    
    Get the specified dynamic indicator parameter
    
    :param str name: the parameter name
    :return: the dynamic indicator parameter
    :rtype: IndParam
    :raises out_of_range: no such parameter)")

      .def("set_ind_param", setIndParam1)
      .def("set_ind_param", setIndParam2, R"(set_ind_param(self, name, ind)

    Set the dynamic indicator parameter

    :param str name: the parameter name
    :param Indicator|IndParam: the parameter value (can be an Indicator or an IndParam instance))")

      .def("empty", &Indicator::empty, "Whether it is empty")
      .def("clone", &Indicator::clone, "The clone operation")
      .def("formula", &Indicator::formula, R"(formula(self)

    Print the indicator formula

    :rtype: str)")

      .def("get_result_num", &Indicator::getResultNumber, R"(get_result_num(self)

    Get the number of the result sets

    :rtype: int)")

      .def("get", &Indicator::get, py::arg("pos"), py::arg("result_index") = 0,
           R"(get(self, pos[, result_index=0])

    Get the value at the specified position.

    This is the speed first entry: the value is read straight from the core
    buffer with no extra work in the binding layer. Normal code should use the
    [] operator (__getitem__), which validates the index and supports negative
    values; get is meant for performance sensitive code (e.g. hot loops) where
    the caller has already ensured 0 <= pos < len(self) and
    0 <= result_index < get_result_num(). Out of range or negative access is
    not guaranteed to behave in any particular way.

    :param int pos: the specified index position
    :param int result_index: the specified result set
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

    Get the index position corresponding to the specified date; if there is no corresponding position, return None

    :param Datetime date: the specified date
    :rtype: int)")

      .def("get_datetime", &Indicator::getDatetime, R"(get_datetime(self, pos)

    Get the date at the specified position

    :param int pos: the specified index position
    :rtype: Datetime)")

      .def("get_by_datetime", &Indicator::getByDate, py::arg("datetime"),
           py::arg("result_index") = 0,
           R"(get_by_datetime(self, datetime[, result_index=0])

    Get the value of the specified date. If there is no result for the corresponding date, return constant.null_price

    :param Datetime datetime: the specified date
    :param int result_index: the specified result set
    :rtype: float)")

      .def("get_result", &Indicator::getResult, R"(get_result(self, result_index)

    Get the specified result set

    :param int result_index: the specified result set
    :rtype: Indicator)")

      .def("get_result_as_price_list", &Indicator::getResultAsPriceList,
           R"(get_result_as_price_list(self, result_index)

    Get the specified result set

    :param int result_index: the specified result set
    :rtype: PriceList)")

      .def("get_datetime_list", &Indicator::getDatetimeList, R"(get_datetime_list(self)

    Return the corresponding date list

    :rtype: DatetimeList)")

      .def("exist_nan", &Indicator::existNan, py::arg("result_idx=0"),
           R"(exist_nan(self, result_idx)

    Judge whether a NaN value exists

    :param int result_idx: the specified result set
    :rtype: bool)")

      .def("set_context",
           [](Indicator& self, const Stock& stock, const KQuery& query) {
               ScopeGilRelease gil(!self.hasPythonPart());
               self.setContext(stock, query);
           })
      .def(
        "set_context",
        [](Indicator& self, const KData& kdata) {
            ScopeGilRelease gil(!self.hasPythonPart());
            self.setContext(kdata);
        },
        R"(set_context(self, kdata)

    Set the context

    :param KData kdata: the associated context K-line)
      
set_context(self, stock, query)

    Set the context

    :param Stock stock: the specified Stock
    :param Query query: the specified query condition)")

      .def("get_context", &Indicator::getContext, R"(get_context(self)

    Get the context

    :rtype: KData)")

      .def(
        "extend",
        [](Indicator& self) {
            ScopeGilRelease gil(!self.hasPythonPart());
            self.extend();
        },
        R"(extend(self)

    When there is a context, automatically extend the context to the current latest data and calculate)")

      .def("contains", &Indicator::contains, R"(contains(self, name)
        
    Get whether the indicator formula contains the indicator with the specified name
    
    :param str name: the specified indicator name
    :rtype: bool)")

      .def("equal", &Indicator::equal)
      .def("is_same", &Indicator::isSame)
      .def("get_imp", &Indicator::getImp)
      .def("__len__", &Indicator::size)

      .def("__call__",
           [](Indicator& self, const Indicator& ind) {
               ScopeGilRelease gil(!self.hasPythonPart() && !ind.hasPythonPart());
               return self(ind);
           })
      .def("__call__",
           [](const Indicator& self, const KData& kdata) {
               ScopeGilRelease gil(!self.hasPythonPart());
               return self(kdata);
           })
      .def("__call__",
           [](Indicator& self) {
               ScopeGilRelease gil(!self.hasPythonPart());
               return self();
           })

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
                       throw std::invalid_argument("Invalid slice parameters");
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
            size_t total = imp->size();

            auto dates = imp->getDatetimeList();
            HKU_CHECK(dates.empty() || total == 0 || dates.size() == total,
                      "The length of the datetime list ({}) does not match the indicator size "
                      "({})!",
                      dates.size(), total);

            std::unique_ptr<uint64_t[]> buffer(new uint64_t[total * (ret_num + 1)]);

            std::vector<string> names;
            std::vector<string> fields;
            std::vector<int64_t> offsets;

            size_t bytes_size;
            if (!dates.empty()) {
                names.push_back("datetime");
                fields.push_back("datetime64[ns]");
                offsets.push_back(0);
                for (size_t i = 0; i < ret_num; i++) {
                    names.push_back(fmt::format("value{}", i));
                    fields.push_back(indicatorValueNpField<Indicator::value_t>());
                    offsets.push_back(offsets.back() + sizeof(Indicator::value_t));
                }
                bytes_size = sizeof(int64_t) + ret_num * sizeof(Indicator::value_t);
            } else {
                for (size_t i = 0; i < ret_num; i++) {
                    names.push_back(fmt::format("value{}", i));
                    fields.push_back(indicatorValueNpField<Indicator::value_t>());
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
                HKU_CHECK(total == 0 || src[i] != nullptr, "The result {} is not ready!", i);
            }

            uint64_t* data = buffer.get();
            auto* val = reinterpret_cast<Indicator::value_t*>(buffer.get());
            if (!dates.empty()) {
                size_t x = ret_num + 1;
                for (size_t i = 0; i < total; i++) {
                    data[i * x] = dates[i].timestamp() * 1000LL;
                    for (size_t j = 0; j < ret_num; j++) {
                        val[i * x + j + 1] = src[j][i];
                    }
                }
            } else if (ret_num == 1) {
                // contiguous layout, bulk copy
                std::memcpy(val, src[0], total * sizeof(Indicator::value_t));
            } else {
                for (size_t i = 0; i < total; i++) {
                    for (size_t j = 0; j < ret_num; j++) {
                        val[i * ret_num + j] = src[j][i];
                    }
                }
            }

            auto capsule = py::capsule(buffer.release(),
                                       [](void* ptr) { delete[] static_cast<uint64_t*>(ptr); });
            ret = py::array(dtype, total, data, capsule);
            return ret;
        },
        "Convert to np.array; if it is a time series, the datetime date column will be included")

      .def(
        "value_to_np",
        [](const Indicator& self) -> py::array {
            size_t ret_num = self.getResultNumber();
            if (ret_num == 0) {
                return py::array_t<Indicator::value_t>((py::ssize_t)0);
            }

            // Initialize the array_t and get its internal buffer
            py::array_t<Indicator::value_t> ret;
            ret.resize({self.size(), ret_num});  // The 2D shape: [size, ret_num]
            auto buf = ret.request();
            auto* buffer =
              static_cast<Indicator::value_t*>(buf.ptr);  // Get the pointer from the array_t

            std::vector<std::string> names;
            std::vector<std::string> fields;
            std::vector<int64_t> offsets;
            for (size_t i = 0; i < ret_num; i++) {
                names.push_back(fmt::format("value{}", i));
                fields.push_back(indicatorValueNpField<Indicator::value_t>());
                offsets.push_back(i *
                                  sizeof(Indicator::value_t));  // Simplify the offset calculation
            }

            auto dtype = py::dtype(
              vector_to_python_list<std::string>(names), vector_to_python_list<std::string>(fields),
              vector_to_python_list<int64_t>(offsets), ret_num * sizeof(Indicator::value_t));

            std::vector<const Indicator::value_t*> src(ret_num);
            for (size_t i = 0; i < ret_num; i++) {
                src[i] = self.data(i);
                HKU_CHECK(self.size() == 0 || src[i] != nullptr, "The result {} is not ready!", i);
            }

            // Fill the data into the buffer of the array_t
            size_t total = self.size();
            if (ret_num == 1) {
                // contiguous layout, bulk copy
                std::memcpy(buffer, src[0], total * sizeof(Indicator::value_t));
            } else {
                for (size_t i = 0; i < total; i++) {
                    for (size_t j = 0; j < ret_num; j++) {
                        buffer[i * ret_num + j] = src[j][i];
                    }
                }
            }

            return py::array(dtype, {self.size()}, {ret_num * sizeof(Indicator::value_t)}, buf.ptr,
                             ret);
        },
        "Convert only the values to np.array, without the date column")

      .def(
        "to_array",
        [](const Indicator& self, size_t result_index) {
            HKU_CHECK(result_index < self.getResultNumber(), "result_index out of range");
            auto ret = py::array_t<Indicator::value_t>(self.size());
            auto buf = ret.request();
            auto* ptr = static_cast<Indicator::value_t*>(buf.ptr);
            const auto* src = self.data(result_index);
            HKU_CHECK(self.size() == 0 || src != nullptr, "The result {} is not ready!",
                      result_index);
            std::memcpy(ptr, src, self.size() * sizeof(Indicator::value_t));
            return ret;
        },
        py::arg("result_index") = 0, "Convert the specified result set to numpy.array")

      .def(
        "to_df",
        [](const Indicator& self) {
            size_t total = self.size();
            if (total == 0) {
                return py::module_::import("pandas").attr("DataFrame")();
            }

            py::dict columns;
            auto dates = self.getDatetimeList();
            HKU_CHECK(dates.empty() || dates.size() == total,
                      "The length of the datetime list ({}) does not match the indicator size "
                      "({})!",
                      dates.size(), total);
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
                py::array_t<Indicator::value_t> arr(total);
                auto buf = arr.request();
                auto* dst = static_cast<Indicator::value_t*>(buf.ptr);
                const auto* src = self.data(i);
                HKU_CHECK(src != nullptr, "The result {} is not ready!", i);
                std::memcpy(dst, src, total * sizeof(Indicator::value_t));
                columns[fmt::format("value{}", i).c_str()] = arr;
            }

            return py::module_::import("pandas").attr("DataFrame")(columns,
                                                                   py::arg("copy") = false);
        },
        "Convert to a DataFrame")

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
                py::array_t<Indicator::value_t> arr(total);
                auto buf = arr.request();
                auto* dst = static_cast<Indicator::value_t*>(buf.ptr);
                const auto* src = self.data(i);
                HKU_CHECK(src != nullptr, "The result {} is not ready!", i);
                std::memcpy(dst, src, total * sizeof(Indicator::value_t));
                columns[fmt::format("value{}", i).c_str()] = arr;
            }

            return py::module_::import("pandas").attr("DataFrame")(columns,
                                                                   py::arg("copy") = false);
        },
        "Convert to a DataFrame, containing only the values")

      .def(+py::self)
      .def(py::self + py::self)
      .def(py::self + Indicator::value_t())
      .def(Indicator::value_t() + py::self)

      .def(-py::self)
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