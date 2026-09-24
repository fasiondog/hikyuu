/*
 * _Slippage.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/slippage/SlippageBase.h>
#include <hikyuu/trade_sys/slippage/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PySlippageBase : public SlippageBase {
    PY_CLONE(PySlippageBase, SlippageBase)

public:
    PySlippageBase() : SlippageBase() {
        m_is_python_object = true;
    }

    PySlippageBase(const string& name) : SlippageBase(name) {
        m_is_python_object = true;
    }

    PySlippageBase(const SlippageBase& base) : SlippageBase(base) {
        m_is_python_object = true;
    }

    void _calculate() override {
        PYBIND11_OVERLOAD_PURE(void, SlippageBase, _calculate, );
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, SlippageBase, _reset, );
    }

    price_t getRealBuyPrice(const Datetime& datetime, price_t planPrice) override {
        PYBIND11_OVERLOAD_PURE_NAME(price_t, SlippageBase, "get_real_buy_price", getRealBuyPrice,
                                    datetime, planPrice);
    }

    price_t getRealSellPrice(const Datetime& datetime, price_t planPrice) override {
        PYBIND11_OVERLOAD_PURE_NAME(price_t, SlippageBase, "get_real_sell_price", getRealSellPrice,
                                    datetime, planPrice);
    }
};

void export_Slippage(py::module& m) {
    py::class_<SlippageBase, SPPtr, PySlippageBase>(m, "SlippageBase", py::dynamic_attr(),
                                                    R"(The slippage algorithm base class

The custom slippage interfaces:

    - getRealBuyPrice : [Required] Calculate the actual buy price
    - getRealSellPrice : [Required] Calculate the actual sell price
    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables)")

      .def(py::init<>())
      .def(py::init<const SlippageBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor
        
    :param str name: the name)")

      .def("__str__", to_py_str<SlippageBase>)
      .def("__repr__", to_py_str<SlippageBase>)

      .def_property("name", py::overload_cast<>(&SlippageBase::name, py::const_),
                    py::overload_cast<const string&>(&SlippageBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("to", &SlippageBase::getTO, &SlippageBase::setTO, "The associated trading object")

      .def("get_param", &SlippageBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (SlippageBase::*)(const std::string&, const boost::any&)>(
             &SlippageBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &SlippageBase::haveParam, "Whether the specified parameter exists")

      .def("get_real_buy_price", &SlippageBase::getRealBuyPrice,
           R"(get_real_buy_price(self, datetime, price)

    [Overload interface] Calculate the actual buy price

    :param Datetime datetime: the buy time
    :param float price: the planned buy price
    :return: the actual buy price
    :rtype: float)")

      .def("get_real_sell_price", &SlippageBase::getRealSellPrice,
           R"(get_real_sell_price(self, datetime, price)

    [Overload interface] Calculate the actual sell price

    :param Datetime datetime: the sell time
    :param float price: the planned sell price
    :return: the actual sell price
    :rtype: float)")

      .def("reset", &SlippageBase::reset, "The reset operation")
      .def("clone", &SlippageBase::clone, "The clone operation")
      .def("_calculate", &SlippageBase::_calculate, "[Overload interface] The subclass calculation interface")
      .def("_reset", &SlippageBase::_reset, "[Overload interface] The subclass reset interface, resetting the internal private variables")

        DEF_PICKLE(SPPtr);

    m.def("SP_FixedPercent", &SP_FixedPercent, py::arg("p") = 0.001,
          R"(SP_FixedPercent([p=0.001])

    The fixed percentage slippage algorithm; the actual buy price = the planned buy price * (1 + p), and the actual sell price = the planned sell price * (1 - p)

    :param float p: the fixed percentage of the offset
    :return: the slippage algorithm instance)");

    m.def("SP_FixedValue", &SP_FixedValue, py::arg("value") = 0.01, R"(SP_FixedValue([value=0.01])

    The fixed price slippage algorithm; the actual buy price = the planned buy price + the offset price, and the actual sell price = the planned sell price - the offset price

    :param float value: the offset price
    :return: the slippage algorithm instance)");

    m.def("SP_Uniform", &SP_Uniform, py::arg("min_value") = -0.05, py::arg("max_value") = 0.05,
          R"(SP_Uniform([min_value=-0.05, max_value=0.05])

    The uniform distribution random price slippage algorithm; the buy and the sell operations are the random price offsets uniformly distributed within the range [min_value, max_value]
    
    :param float min_value: the minimum offset price
    :param float max_value: the maximum offset price
    :return: the slippage algorithm instance)");

    m.def("SP_Normal", &SP_Normal, py::arg("mean") = 0.0, py::arg("stddev") = 0.05,
          R"(SP_Normal([mean=0.0, stddev=0.05])

    The normal distribution random price slippage algorithm; the buy and the sell operations are the random price offsets within the normal distribution [mean, stddev] range
    
    :param float mean: the mean of the normal distribution
    :param float stddev: the standard deviation of the normal distribution
    :return: the slippage algorithm instance)");

    m.def("SP_LogNormal", &SP_LogNormal, py::arg("mean") = 0.0, py::arg("stddev") = 0.05,
          R"(SP_LogNormal([mean=0.0, stddev=0.05])

    The log-normal distribution random price slippage algorithm; the buy and the sell operations are the random price offsets within the log-normal distribution [mean, stddev] range

    :param float mean: the mean of the log-normal distribution
    :param float stddev: the standard deviation of the log-normal distribution
    :return: the slippage algorithm instance)");

    m.def("SP_TruncNormal", &SP_TruncNormal, py::arg("mean") = 0.0, py::arg("stddev") = 0.05,
          py::arg("min_value") = -0.11, py::arg("max_value") = 0.1,
          R"(SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1])
         
    The truncated normal distribution random price slippage algorithm; the buy and the sell operations are the random price offsets within the truncated normal distribution [mean, stddev, min_value, max_value] range
    
    :param float mean: the mean of the truncated normal distribution
    :param float stddev: the standard deviation of the truncated normal distribution
    :param float min_value: the minimum truncation value
    :param float max_value: the maximum truncation value
    :return: the slippage algorithm instance)");
}