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

    price_t _getRealBuyShortPrice(const Datetime& datetime, price_t planPrice) override {
        PYBIND11_OVERLOAD_NAME(price_t, SlippageBase, "_get_real_buy_short_price",
                               _getRealBuyShortPrice, datetime, planPrice);
    }

    price_t _getRealSellShortPrice(const Datetime& datetime, price_t planPrice) override {
        PYBIND11_OVERLOAD_NAME(price_t, SlippageBase, "_get_real_sell_short_price",
                               _getRealSellShortPrice, datetime, planPrice);
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
    - _reset : [Optional] Reload the private variables
    - _get_real_buy_short_price : [Optional] Compute the raw price of covering a short, overriding _get_real_buy_price on the short side
    - _get_real_sell_short_price : [Optional] Compute the raw price of opening a short, overriding _get_real_sell_price on the short side)")

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

      .def("get_real_buy_short_price", &SlippageBase::getRealBuyShortPrice,
           R"(get_real_buy_short_price(self, datetime, price)

    [Public method] Return the raw price used to cover a short position by calling the subclass hook
    _get_real_buy_short_price, which falls back to _get_real_buy_price when that hook is not
    overridden. The System part validates the result the same way as get_real_buy_price does.

    :param Datetime datetime: the buy time
    :param float price: the planned buy price
    :return: the raw buy price of the slippage model for covering a short
    :rtype: float)")

      .def("get_real_sell_short_price", &SlippageBase::getRealSellShortPrice,
           R"(get_real_sell_short_price(self, datetime, price)

    [Public method] Return the raw price used to open a short position by calling the subclass hook
    _get_real_sell_short_price, which falls back to _get_real_sell_price when that hook is not
    overridden.

    :param Datetime datetime: the sell time
    :param float price: the planned sell price
    :return: the raw sell price of the slippage model for opening a short
    :rtype: float)")

      .def("_get_real_buy_short_price", &SlippageBase::_getRealBuyShortPrice,
           R"(_get_real_buy_short_price(self, datetime, price)

    [Override hook] Compute the raw price of covering a short. Override it only when the short side
    should slip differently from the long side; the default delegates to _get_real_buy_price.

    :param Datetime datetime: the buy time
    :param float price: the planned buy price
    :return: the raw buy price for covering a short
    :rtype: float)")

      .def("_get_real_sell_short_price", &SlippageBase::_getRealSellShortPrice,
           R"(_get_real_sell_short_price(self, datetime, price)

    [Override hook] Compute the raw price of opening a short. Override it only when the short side
    should slip differently from the long side; the default delegates to _get_real_sell_price.

    :param Datetime datetime: the sell time
    :param float price: the planned sell price
    :return: the raw sell price for opening a short
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
          py::arg("short_p") = py::none(),
          R"(SP_FixedPercent([p=0.001, short_p=None])

    The fixed percentage slippage algorithm; the actual buy price = the planned buy price * (1 + p), and the actual sell price = the planned sell price * (1 - p). p must be in [0, 1).

    :param float p: the fixed percentage of the offset
    :param float short_p: the fixed percentage used when opening a short and covering it, must be in [0, 1); None (default) takes the same value as p
    :return: the slippage algorithm instance)");

    m.def("SP_FixedValue", &SP_FixedValue, py::arg("value") = 0.01,
          py::arg("short_value") = py::none(),
          R"(SP_FixedValue([value=0.01, short_value=None])

    The fixed price slippage algorithm; the actual buy price = the planned buy price + the offset price, and the actual sell price = the planned sell price - the offset price

    :param float value: the offset price, must be >= 0
    :param float short_value: the offset price used when opening a short and covering it, must be >= 0; None (default) takes the same value as value
    :return: the slippage algorithm instance)");

    m.def(
      "SP_Uniform", &SP_Uniform, py::arg("min_value") = -0.05, py::arg("max_value") = 0.05,
      py::arg("adverse_only") = true, py::arg("short_min_value") = py::none(),
      py::arg("short_max_value") = py::none(),
      R"(SP_Uniform([min_value=-0.05, max_value=0.05, adverse_only=True, short_min_value=None, short_max_value=None])

    The uniform distribution random price slippage algorithm. When adverse_only is True (default), the sampled value is folded to a non-negative amplitude and applied in the unfavorable direction (one-sided cost model). When adverse_only is False, the raw signed value is applied directly as zero-mean execution noise (not necessarily symmetric; zero mean when the range is centered at 0).

    :param float min_value: the minimum of the offset range
    :param float max_value: the maximum of the offset range, must be >= min_value
    :param bool adverse_only: True (default) folds to unfavorable only; False uses raw signed offset
    :param float short_min_value: the minimum of the short-side offset range; None (default) takes min_value
    :param float short_max_value: the maximum of the short-side offset range; None (default) takes max_value
    :return: the slippage algorithm instance

    Besides the arguments above, the instance carries a seed parameter (int64, default 0 = a random seed); a non-zero value makes the sequence reproducible, and it takes effect only after reset(). The long and the short hooks draw from the same engine, so the sequence depends on the order of the calls.)");

    m.def(
      "SP_Normal", &SP_Normal, py::arg("mean") = 0.0, py::arg("stddev") = 0.05,
      py::arg("adverse_only") = true, py::arg("short_mean") = py::none(),
      py::arg("short_stddev") = py::none(),
      R"(SP_Normal([mean=0.0, stddev=0.05, adverse_only=True, short_mean=None, short_stddev=None])

    The normal distribution random price slippage algorithm. When adverse_only is True (default), the sampled value is folded to a non-negative amplitude and applied in the unfavorable direction, so the actual expected slippage is the folded expectation E|X|, not the mean. When adverse_only is False, the raw signed value is applied directly as zero-mean execution noise (not necessarily symmetric; its expectation is mean).

    :param float mean: the mean of the normal distribution
    :param float stddev: the standard deviation of the normal distribution, must be >= 0
    :param bool adverse_only: True (default) folds to unfavorable only; False uses raw signed offset
    :param float short_mean: the mean used when opening a short and covering it; None (default) takes mean
    :param float short_stddev: the standard deviation used when opening a short and covering it; None (default) takes stddev
    :return: the slippage algorithm instance

    Besides the arguments above, the instance carries a seed parameter (int64, default 0 = a random seed); a non-zero value makes the sequence reproducible, and it takes effect only after reset(). The long and the short hooks draw from the same engine, so the sequence depends on the order of the calls.)");

    m.def(
      "SP_LogNormal", &SP_LogNormal, py::arg("mean") = 0.0, py::arg("stddev") = 0.05,
      py::arg("adverse_only") = true, py::arg("short_mean") = py::none(),
      py::arg("short_stddev") = py::none(),
      R"(SP_LogNormal([mean=0.0, stddev=0.05, adverse_only=True, short_mean=None, short_stddev=None])

    The log-normal distribution random price slippage algorithm. When adverse_only is True (default), the centered sampled value is folded to a non-negative amplitude and applied in the unfavorable direction. When adverse_only is False, the raw centered value (X - E[X]) is applied directly as zero-mean execution noise: its expectation is exactly 0, but it is right-skewed and not necessarily symmetric, and mean only raises the dispersion without producing a drift (unlike mean in SP_Normal).

    :param float mean: the mean of the log-normal distribution
    :param float stddev: the standard deviation of the log-normal distribution, must be >= 0
    :param bool adverse_only: True (default) folds to unfavorable only; False uses raw signed offset
    :param float short_mean: the mean used when opening a short and covering it; None (default) takes mean
    :param float short_stddev: the standard deviation used when opening a short and covering it; None (default) takes stddev
    :return: the slippage algorithm instance

    Besides the arguments above, the instance carries a seed parameter (int64, default 0 = a random seed); a non-zero value makes the sequence reproducible, and it takes effect only after reset(). The long and the short hooks draw from the same engine, so the sequence depends on the order of the calls.)");

    m.def(
      "SP_TruncNormal", &SP_TruncNormal, py::arg("mean") = 0.0, py::arg("stddev") = 0.05,
      py::arg("min_value") = -0.1, py::arg("max_value") = 0.1, py::arg("adverse_only") = true,
      py::arg("short_mean") = py::none(), py::arg("short_stddev") = py::none(),
      py::arg("short_min_value") = py::none(), py::arg("short_max_value") = py::none(),
      R"(SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1, adverse_only=True, short_mean=None, short_stddev=None, short_min_value=None, short_max_value=None])

    The truncated normal distribution random price slippage algorithm. When adverse_only is True (default), the sampled value is folded to a non-negative amplitude and applied in the unfavorable direction. When adverse_only is False, the raw signed value is applied directly as zero-mean execution noise (not necessarily symmetric; the expectation equals mean only when the truncation interval is centered on it). The sample is drawn from the truncated normal distribution over [min_value, max_value].

    :param float mean: the mean of the truncated normal distribution
    :param float stddev: the standard deviation of the truncated normal distribution, must be >= 0
    :param float min_value: the minimum truncation value
    :param float max_value: the maximum truncation value, must be >= min_value
    :param bool adverse_only: True (default) folds to unfavorable only; False uses raw signed offset
    :param float short_mean: the mean used when opening a short and covering it; None (default) takes mean
    :param float short_stddev: the standard deviation used when opening a short and covering it; None (default) takes stddev
    :param float short_min_value: the minimum short-side truncation value; None (default) takes min_value
    :param float short_max_value: the maximum short-side truncation value; None (default) takes max_value
    :return: the slippage algorithm instance

    Besides the arguments above, the instance carries a seed parameter (int64, default 0 = a random seed); a non-zero value makes the sequence reproducible, and it takes effect only after reset(). The long and the short hooks draw from the same engine, so the sequence depends on the order of the calls.)");
}
