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

    price_t _getRealBuyPrice(const Datetime& datetime, price_t planPrice) override {
        PYBIND11_OVERLOAD_PURE_NAME(price_t, SlippageBase, "_get_real_buy_price", _getRealBuyPrice,
                                    datetime, planPrice);
    }

    price_t _getRealSellPrice(const Datetime& datetime, price_t planPrice) override {
        PYBIND11_OVERLOAD_PURE_NAME(price_t, SlippageBase, "_get_real_sell_price",
                                    _getRealSellPrice, datetime, planPrice);
    }
};

void export_Slippage(py::module& m) {
    py::class_<SlippageBase, SPPtr, PySlippageBase>(m, "SlippageBase", py::dynamic_attr(),
                                                    R"(The slippage algorithm base class

The custom slippage interfaces:

    - _get_real_buy_price : [Required] Compute the raw buy price (the System part validates it, snaps it to the market's minimum price tick and clamps it into the bar's [low, high] range)
    - _get_real_sell_price : [Required] Compute the raw sell price (the System part validates it, snaps it to the market's minimum price tick and clamps it into the bar's [low, high] range)
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
      .def_property("to", &SlippageBase::getTO, &SlippageBase::setTO,
                    "The associated trading object")

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

    [Public method] Return the raw buy price of the slippage model by calling the subclass hook
    _get_real_buy_price. The System part is the only caller in the framework: it validates the
    result (an invalid value falls back to the plan price), snaps it to the market's minimum
    price tick and clamps it into the bar's [low, high] range before the trade. Do not override
    this method; override _get_real_buy_price instead.

    :param Datetime datetime: the buy time
    :param float price: the planned buy price
    :return: the raw buy price of the slippage model
    :rtype: float)")

      .def("get_real_sell_price", &SlippageBase::getRealSellPrice,
           R"(get_real_sell_price(self, datetime, price)

    [Public method] Return the raw sell price of the slippage model by calling the subclass hook
    _get_real_sell_price. The System part is the only caller in the framework: it validates the
    result (an invalid value falls back to the plan price), snaps it to the market's minimum
    price tick and clamps it into the bar's [low, high] range before the trade. Do not override
    this method; override _get_real_sell_price instead.

    :param Datetime datetime: the sell time
    :param float price: the planned sell price
    :return: the raw sell price of the slippage model
    :rtype: float)")

      .def("_get_real_buy_price", &SlippageBase::_getRealBuyPrice,
           R"(_get_real_buy_price(self, datetime, price)

    [Override hook] Compute the raw buy price. The slippage algorithm only produces the price of
    the model: the validity check and the day-range clamp are applied by the System part, which
    is the only caller in the framework.

    :param Datetime datetime: the buy time
    :param float price: the planned buy price
    :return: the raw buy price of the slippage model
    :rtype: float)")

      .def("_get_real_sell_price", &SlippageBase::_getRealSellPrice,
           R"(_get_real_sell_price(self, datetime, price)

    [Override hook] Compute the raw sell price. The slippage algorithm only produces the price of
    the model: the validity check and the day-range clamp are applied by the System part, which
    is the only caller in the framework.

    :param Datetime datetime: the sell time
    :param float price: the planned sell price
    :return: the raw sell price of the slippage model
    :rtype: float)")

      .def("reset", &SlippageBase::reset, "The reset operation")
      .def("clone", &SlippageBase::clone, "The clone operation")
      .def("_calculate", &SlippageBase::_calculate,
           "[Overload interface] The subclass calculation interface. Do not call "
           "super()._calculate() in a Python override (causes infinite recursion).")
      .def("_reset", &SlippageBase::_reset,
           "[Overload interface] The subclass reset interface, resetting the internal private "
           "variables")

        DEF_PICKLE(SPPtr);

    m.def("SP_FixedPercent", &SP_FixedPercent, py::arg("p") = 0.001,
          R"(SP_FixedPercent([p=0.001])

    The fixed percentage slippage algorithm; the actual buy price = the planned buy price * (1 + p), and the actual sell price = the planned sell price * (1 - p). p must be in [0, 1).

    :param float p: the fixed percentage of the offset
    :return: the slippage algorithm instance)");

    m.def("SP_FixedValue", &SP_FixedValue, py::arg("value") = 0.01, R"(SP_FixedValue([value=0.01])

    The fixed price slippage algorithm; the actual buy price = the planned buy price + the offset price, and the actual sell price = the planned sell price - the offset price

    :param float value: the offset price, must be >= 0
    :return: the slippage algorithm instance)");

    m.def("SP_Uniform", &SP_Uniform, py::arg("min_value") = -0.05, py::arg("max_value") = 0.05,
          R"(SP_Uniform([min_value=-0.05, max_value=0.05])

    The uniform distribution random price slippage algorithm. The sampled value is folded to a non-negative amplitude and applied in the unfavorable direction, so the buy price is the plan price plus the amplitude and the sell price is the plan price minus the amplitude. [min_value, max_value] is the amplitude range (the sign of the sample is ignored), not a symmetric +/- offset.

    :param float min_value: the minimum of the offset amplitude range
    :param float max_value: the maximum of the offset amplitude range, must be >= min_value
    :return: the slippage algorithm instance

    Besides the arguments above, the instance carries a seed parameter (int64, default 0 = a random seed); a non-zero value makes the sequence reproducible, and it takes effect only after reset().)");

    m.def("SP_Normal", &SP_Normal, py::arg("mean") = 0.0, py::arg("stddev") = 0.05,
          R"(SP_Normal([mean=0.0, stddev=0.05])

    The normal distribution random price slippage algorithm. The sampled value is folded to a non-negative amplitude and applied in the unfavorable direction, so the buy price is the plan price plus the amplitude and the sell price is the plan price minus the amplitude. The actual expected slippage is therefore the folded expectation E|X|, not the mean:

        E|X| = stddev * sqrt(2/pi) * exp(-mean^2 / (2 * stddev^2)) + mean * erf(mean / (stddev * sqrt(2)))

    With mean = 0, E|X| = stddev * sqrt(2/pi), i.e. about 0.7979 * stddev: SP_Normal(0.0, 0.05) gives an expected slippage of about 4%. The mean only becomes visible when it is comparable to stddev (SP_Normal(0.05, 0.05) gives about 5.8%); as it grows far beyond stddev, E|X| approaches the mean - the mean only shifts the center of the distribution before folding.

    :param float mean: the mean of the normal distribution (the center before folding; not the actual expected slippage)
    :param float stddev: the standard deviation of the normal distribution, must be >= 0
    :return: the slippage algorithm instance

    Besides the arguments above, the instance carries a seed parameter (int64, default 0 = a random seed); a non-zero value makes the sequence reproducible, and it takes effect only after reset().)");

    m.def("SP_LogNormal", &SP_LogNormal, py::arg("mean") = 0.0, py::arg("stddev") = 0.05,
          R"(SP_LogNormal([mean=0.0, stddev=0.05])

    The log-normal distribution random price slippage algorithm. The sampled value is folded to a non-negative amplitude and applied in the unfavorable direction, so the buy price is the plan price plus the amplitude and the sell price is the plan price minus the amplitude.

    :param float mean: the mean of the log-normal distribution
    :param float stddev: the standard deviation of the log-normal distribution, must be >= 0
    :return: the slippage algorithm instance

    Besides the arguments above, the instance carries a seed parameter (int64, default 0 = a random seed); a non-zero value makes the sequence reproducible, and it takes effect only after reset().)");

    m.def("SP_TruncNormal", &SP_TruncNormal, py::arg("mean") = 0.0, py::arg("stddev") = 0.05,
          py::arg("min_value") = -0.1, py::arg("max_value") = 0.1,
          R"(SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1])

    The truncated normal distribution random price slippage algorithm. The sampled value is folded to a non-negative amplitude and applied in the unfavorable direction, so the buy price is the plan price plus the amplitude and the sell price is the plan price minus the amplitude. The sample is drawn from the truncated normal distribution over [min_value, max_value].

    :param float mean: the mean of the truncated normal distribution
    :param float stddev: the standard deviation of the truncated normal distribution, must be >= 0
    :param float min_value: the minimum truncation value
    :param float max_value: the maximum truncation value, must be >= min_value
    :return: the slippage algorithm instance

    Besides the arguments above, the instance carries a seed parameter (int64, default 0 = a random seed); a non-zero value makes the sequence reproducible, and it takes effect only after reset().)");
}
