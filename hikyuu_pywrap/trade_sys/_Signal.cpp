/*
 * _Signal.cpp
 *
 *  Created on: 2013-3-18
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/signal/build_in.h>
#include "../gil_policy.h"
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PySignalBase : public SignalBase {
    PY_CLONE(PySignalBase, SignalBase)

public:
    PySignalBase() : SignalBase() {
        m_is_python_object = true;
    }

    PySignalBase(const string& name) : SignalBase(name) {
        m_is_python_object = true;
    }

    PySignalBase(const SignalBase& base) : SignalBase(base) {
        m_is_python_object = true;
    }

    void _calculate(const KData& kdata) override {
        PYBIND11_OVERLOAD_PURE(void, SignalBase, _calculate, kdata);
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, SignalBase, _reset, );
    }
};

void export_Signal(py::module& m) {
    py::class_<SignalBase, SGPtr, PySignalBase>(m, "SignalBase", py::dynamic_attr(),
                                                R"(The signal generator base class
    The signal generator is responsible for generating the buy and the sell signals.

Common parameters:

    - alternate (bool|True): whether the buy and the sell signals appear alternately. The single-line signals usually judge the generation of the signals through the inflection points, the slopes, etc.; in this case, the consecutive buy signals or the consecutive sell signals may appear, and this parameter can control whether the buy and the sell signals appear alternately. The double-line cross signals usually have the buys and the sells already alternating, in which case this parameter is invalid.

The custom signal generator interfaces:

    - _calculate : [Required] the subclass calculation interface
    - _clone : [Required] the clone interface
    - _reset : [Optional] reload the private variables)")

      .def(py::init<>())
      .def(py::init<const string&>())
      .def(py::init<const SignalBase&>())

      .def("__str__", to_py_str<SignalBase>)
      .def("__repr__", to_py_str<SignalBase>)

      .def_property("name", py::overload_cast<>(&SignalBase::name, py::const_),
                    py::overload_cast<const string&>(&SignalBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property(
        "to", &SignalBase::getTO,
        [](SignalBase* self, const KData& kdata) {
            ScopeGilRelease gil(!self->hasPythonPart());
            self->setTO(kdata);
        },
        py::return_value_policy::copy, "Set or get the trading object")

      .def("get_cycle_dates", &SignalBase::getCycleDates, py::return_value_policy::copy,
           R"(get_cycle_dates(self)

    Get the rebalancing days declared by the MultiSystem. A custom cycle signal reads them in
    _calculate to know on which days the scheduled signals must be emitted.

    :return: DatetimeList)")

      .def(
        "set_cycle_dates",
        [](SignalBase* self, const py::object& dates) {
            // Convert before releasing the GIL: the iterator and the cast both need it
            auto cycle_dates = get_datetime_list_from_python(dates);
            ScopeGilRelease gil(!self->hasPythonPart());
            self->setCycleDates(cycle_dates);
        },
        py::arg("dates"),
        R"(set_cycle_dates(self, dates)

    Declare the rebalancing days consumed by the cycle signals, usually injected by a MultiSystem
    before the run; the other signals ignore them. The dates are sorted and deduplicated, and the
    signal is recalculated at once when the trading object is already set.

    :param dates: the Datetime sequence (list/tuple/DatetimeList are all accepted))")

      .def("get_param", &SignalBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (SignalBase::*)(const std::string&, const boost::any&)>(
             &SignalBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &SignalBase::haveParam, "Whether the specified parameter exists")

      .def("should_buy", &SignalBase::shouldBuy, R"(should_buy(self, datetime)

    Whether it can be bought at the specified moment

    :param Datetime datetime: the specified moment
    :rtype: bool)")

      .def("should_sell", &SignalBase::shouldSell, R"(should_sell(self, datetime)

    Whether it can be sold at the specified moment

    :param Datetime datetime: the specified moment
    :rtype: bool)")

      .def("next_time_should_buy", &SignalBase::nextTimeShouldBuy,
           R"(next_time_should_buy(self)

    Whether it can be bought at the next moment, equivalent to whether the last moment indicates buying)")

      .def("next_time_should_sell", &SignalBase::nextTimeShouldSell, R"(next_time_should_sell(self)
      
    Whether it can be sold at the next moment, equivalent to whether the last moment indicates selling)")

      .def("get_buy_signal", &SignalBase::getBuySignal, R"(get_buy_signal(self)

    Get the list of all the buy indication dates
    
    :rtype: DatetimeList)")

      .def("get_sell_signal", &SignalBase::getSellSignal, R"(get_sell_signal(self)

    Get the list of all the sell indication dates

    :rtype: DatetimeList)")

      .def("get_buy_value", &SignalBase::getBuyValue, py::arg("datetime"),
           R"(get_buy_value(self, datetime)

    Get the buy signal value at the specified datetime; a value less than or equal to 0 means no
    buy signal

    :param Datetime datetime: the specified datetime
    :rtype: float)")

      .def("get_sell_value", &SignalBase::getSellValue, py::arg("datetime"),
           R"(get_sell_value(self, datetime)

    Get the sell signal value at the specified datetime; a value greater than or equal to 0 means
    no sell signal

    :param Datetime datetime: the specified datetime
    :rtype: float)")

      .def("get_value", &SignalBase::getValue, py::arg("datetime"),
           R"(get_value(self, datetime)

    Get the net signal value at the specified datetime, the sum of the buy and sell values:
    positive means a buy intent, negative a sell intent, and about zero means no signal

    :param Datetime datetime: the specified datetime
    :rtype: float)")

      .def("_add_signal", &SignalBase::_addSignal, py::arg("datetime"), py::arg("value"),
           R"(_add_signal(self, datetime, value)

    Add a signal value, called in _calculate.

    The signals of the same date are merged into one net value: a positive net keeps only the buy
    signal, a negative net keeps only the sell signal (the opposite record of that date is
    dropped), and a net of about zero leaves no signal on that date (the records already written
    for it are removed).

    :param Datetime datetime: the date the signal occurs
    :param float value: the signal value, positive for a buy intent, negative for a sell intent)")

      .def("_add_buy_signal", &SignalBase::_addBuySignal, py::arg("datetime"),
           py::arg("value") = 1.0,
           R"(_add_buy_signal(self, datetime, value=1.0)

    Add a buy signal, called in _calculate

    :param Datetime datetime: the date indicating buying
    :param float value: the signal value, must be greater than 0; it is merged with the same-date
                        signals into one net value, so an existing sell signal of the same date is
                        included and only the net direction is kept (a net of about zero drops the
                        records of that date))")

      .def("_add_sell_signal", &SignalBase::_addSellSignal, py::arg("datetime"),
           py::arg("value") = -1.0,
           R"(_add_sell_signal(self, datetime, value=-1.0)

    Add a sell signal, called in _calculate

    :param Datetime datetime: the date indicating selling
    :param float value: the signal value, must be less than 0; it is merged with the same-date
                        signals into one net value, so an existing buy signal of the same date is
                        included and only the net direction is kept (a net of about zero drops the
                        records of that date))")

      .def("reset", &SignalBase::reset, "The reset operation")
      .def("clone", &SignalBase::clone, "The clone operation")
      .def("_calculate", &SignalBase::_calculate, R"(_calculate(self, kdata)
      
    [Overload interface] The subclass calculation interface.
    Do not call super()._calculate() in a Python override (causes infinite recursion).)")

      .def("_reset", &SignalBase::_reset,
           "[Overload interface] The subclass reset interface, resetting the internal private "
           "variables")

      .def("__add__", [](const SignalPtr& self, const SignalPtr& other) { return self + other; })
      .def("__add__", [](const SignalPtr& self, double other) { return self + other; })
      .def("__radd__", [](const SignalPtr& self, double other) { return other + self; })
      .def("__sub__", [](const SignalPtr& self, const SignalPtr& other) { return self - other; })
      .def("__sub__", [](const SignalPtr& self, double other) { return self - other; })
      .def("__rsub__", [](const SignalPtr& self, double other) { return other - self; })
      .def("__mul__", [](const SignalPtr& self, const SignalPtr& other) { return self * other; })
      .def("__mul__", [](const SignalPtr& self, double other) { return self * other; })
      .def("__rmul__", [](const SignalPtr& self, double other) { return other * self; })
      .def("__truediv__",
           [](const SignalPtr& self, const SignalPtr& other) { return self / other; })
      .def("__truediv__", [](const SignalPtr& self, double other) { return self / other; })
      .def("__rtruediv__", [](const SignalPtr& self, double other) { return other / self; })
      .def("__and__", [](const SignalPtr& self, const SignalPtr& other) { return self & other; })
      .def("__or__", [](const SignalPtr& self, const SignalPtr& other) { return self | other; })

        DEF_PICKLE(SGPtr);

    m.def("SG_Bool", SG_Bool, py::arg("buy"), py::arg("sell"), py::arg("alternate") = true,
          R"(SG_Bool(buy, sell)

    The boolean signal generator, using the Indicators whose operation results are like bool arrays as the buy and the sell indications respectively.

    :param Indicator buy: the buy indication (if the corresponding position in the result Indicator is >0, it means buying)
    :param Indicator sell: the sell indication (if the corresponding position in the result Indicator is >0, it means selling)
    :param bool alternate: whether to buy and sell alternately, defaulting to True
    :return: the signal generator)");

    m.def("SG_Single", SG_Single, py::arg("ind"), py::arg("filter_n") = 20,
          py::arg("filter_p") = 0.1,
          R"(SG_Single(ind[, filter_n = 20, filter_p = 0.1])
    
    Generate the single-line inflection point signal generator. It uses the curve inflection point algorithm given in Trade Your Way to Financial Freedom [BOOK1]_ to judge the curve trend; the formula is as follows::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter 
    
    :param Indicator ind: the input indicator
    :param int filter_n: the N-day period
    :param float filter_p: the filter percentage
    :return: the signal generator)");

    m.def("SG_Single2", SG_Single2, py::arg("ind"), py::arg("filter_n") = 20,
          py::arg("filter_p") = 0.1,
          R"(SG_Single2(ind[, filter_n = 20, filter_p = 0.1])
    
    Generate the single-line inflection point signal generator 2 [BOOK1]_::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - @lowest(AMA,n) > filter
        Sell When @highest(AMA, n) - AMA > filter
    
    :param Indicator ind: the input indicator
    :param int filter_n: the N-day period
    :param float filter_p: the filter percentage
    :return: the signal generator)");

    m.def("SG_Cross", SG_Cross, py::arg("fast"), py::arg("slow"),
          R"(SG_Cross(fast, slow)

    The double-line cross indicator; when the fast line crosses the slow line from below upward, buy; when the fast line crosses the slow line from above downward, sell. E.g.: buy when the 5-day MA crosses above the 10-day MA, and sell when the 5-day MA crosses below the 10-day MA:: 

        SG_Cross(MA(C, n=10), MA(C, n=30))

    :param Indicator fast: the fast line
    :param Indicator slow: the slow line
    :return: the signal generator)");

    m.def("SG_CrossGold", SG_CrossGold, py::arg("fast"), py::arg("slow"),
          R"(SG_CrossGold(fast, slow)

    The golden cross indicator; when the fast line crosses the slow line from below upward and both the fast line and the slow line are heading upward, it is a golden cross, buy;
    when the fast line crosses the slow line from above downward and both the fast line and the slow line are heading downward, it is a death cross, sell. ::

        SG_CrossGold(MA(C, n=10), MA(C, n=30))

    :param Indicator fast: the fast line
    :param Indicator slow: the slow line
    :return: the signal generator)");

    m.def("SG_Flex", SG_Flex, py::arg("ind"), py::arg("slow_n"),
          R"(SG_Flex(ind, slow_n)

    Use its own EMA(slow_n) as the slow line and itself as the fast line; buy when the fast line crosses the slow line upward, and sell when the fast line crosses the slow line downward.

    :param Indicator ind: the input indicator
    :param int slow_n: the EMA period of the slow line
    :return: the signal generator)");

    m.def("SG_Band",
          py::overload_cast<const Indicator&, const Indicator&, const Indicator&>(SG_Band),
          py::arg("ind"), py::arg("lower"), py::arg("upper"));
    m.def("SG_Band", py::overload_cast<const Indicator&, price_t, price_t>(SG_Band), py::arg("ind"),
          py::arg("lower"), py::arg("upper"),
          R"(SG_Band(ind, lower, upper)
          
    The indicator range indicator; when the indicator exceeds the upper band, buy;
    when the indicator is below the lower band, sell. ::

        SG_Band(MA(C, n=10), 100, 200)
        SG_Band(CLOSE, MA(LOW), MA(HIGH)))");

    m.def("SG_AllwaysBuy", SG_AllwaysBuy, R"(SG_AllwaysBuy()
    
    A special SG that issues the buy signal every day continuously, usually used together with the PF)");

    m.def("SG_Cycle", SG_Cycle, R"(SG_Cycle()
    
    A special SG that emits a buy signal on the rebalancing days injected by a MultiSystem before
    the run; it stays silent when no day is injected. A day without a bar of the instrument (a
    halt) keeps the signal but nothing can fill it, it is not shifted to the next bar)");

    m.def("SG_Manual", SG_Manual, R"(SG_Manual()

    A signal generator whose signals are added manually, for tests or the external signal import.
    Its _calculate is empty: the preset signals are its input and are kept both when it is used
    directly and as a sub-signal of an operator, assigning the trading object or resetting does not
    clear them)");

    m.def("SG_OneSide", SG_OneSide, py::arg("ind"), py::arg("is_buy"),
          R"(SG_OneSide(ind, is_buy)
          
    Build the one-sided signal (containing only the buy or the sell signal) from the input indicator; if the indicator value is greater than 0, add the signal. The SG_Buy or the SG_Sell functions can also be used.
    
    :param Indicator ind: the input indicator
    :param bool is_buy: what is built is the buy signal; otherwise, it is the sell signal
    :return: the signal generator)");

    m.def("SG_Buy", SG_Buy, py::arg("ind"), R"(SG_Buy(ind)
    
    Generate the one-sided buy signal

    :param Indicator ind: the input indicator
    :return: the signal generator)");

    m.def("SG_Sell", SG_Sell, py::arg("ind"), R"(SG_Sell(ind)
    
    Generate the one-sided sell signal

    :param Indicator ind: the input indicator
    :return: the signal generator)");

    m.def(
      "SG_Add",
      [](const py::sequence& sg_list, bool alternate) {
          vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
          return SG_Add(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
    m.def("SG_Add", py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Add),
          py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
          R"(SG_Add(sg1, sg2, alternate)

    Generate the signal of the sum of the two indicators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True
    :return: the signal generator)");

    m.def(
      "SG_Sub",
      [](const py::sequence& sg_list, bool alternate) {
          vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
          return SG_Sub(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
    m.def("SG_Sub", py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Sub),
          py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
          R"(SG_Sub(sg1, sg2, alternate)

    Generate the signal of the difference of the two indicators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True
    :return: the signal generator)");

    m.def(
      "SG_Mul",
      [](const py::sequence& sg_list, bool alternate) {
          vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
          return SG_Mul(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
    m.def("SG_Mul", py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Mul),
          py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
          R"(SG_Mul(sg1, sg2, alternate)

    Generate the signal of the product of the two signal generators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True)");

    m.def(
      "SG_Div",
      [](const py::sequence& sg_list, bool alternate) {
          vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
          return SG_Div(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
    m.def("SG_Div", py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Div),
          py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
          R"(SG_Div(sg1, sg2, alternate)

    Generate the signal of the quotient of the two signal generators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True)");

    m.def(
      "SG_And",
      [](const py::sequence& sg_list, bool alternate) {
          vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
          return SG_And(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
    m.def("SG_And", py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_And),
          py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
          R"(SG_And(sg1, sg2, alternate)

    Generate the signal of the AND of the two indicators

    Only the direction is kept: a hit is written as 1.0 for a buy and -1.0 for a sell, dropping the
    amplitudes of the sub-signals

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True)");

    m.def(
      "SG_Or",
      [](const py::sequence& sg_list, bool alternate) {
          vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
          return SG_Or(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
    m.def("SG_Or", py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Or),
          py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
          R"(SG_Or(sg1, sg2, alternate)

    Generate the signal of the logical OR of the two signal generators

    Only the direction is kept: a hit is written as 1.0 for a buy and -1.0 for a sell, dropping the
    amplitudes of the sub-signals; on a same-day conflict the sell wins

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True)");
}