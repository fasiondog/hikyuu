/*
 * _build_in.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/build_in.h>
#include <hikyuu/trade_manage/tradecost/crt/TC_TestStub.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_build_in(py::module& m) {
    m.def(
      "crtTM", &crtTM, py::arg("date") = Datetime(199001010000LL), py::arg("init_cash") = 100000,
      py::arg("cost_func") = TC_Zero(), py::arg("name") = "SYS", py::arg("margin") = MarginPtr(),
      R"(crtTM([date = Datetime(199001010000), init_cash = 100000, cost_func = TC_Zero(), name = "SYS", margin = None])

    Create the trade manager module, managing the trade records and the fund usage of the account
    
    :param Datetime date:  the account establishment date
    :param float init_cash:    the initial capital
    :param TradeCost cost_func: the trade cost algorithm
    :param string name:        the account name
    :param MarginBase margin:  the margin model; None keeps the account default model MG_Fixed(0.6)
    :rtype: TradeManager)");

    m.def("TC_TestStub", TC_TestStub, "For testing only");

    m.def(
      "TC_FixedA", TC_FixedA, py::arg("commission") = 0.0018, py::arg("lowest_commission") = 5.0,
      py::arg("stamptax") = 0.0005, py::arg("transferfee") = 0.00001,
      R"(TC_FixedA([commission=0.0018, lowest_commission=5.0, stamptax=0.0005, transferfee=0.00001])

    The A-share trade cost algorithm at the current rates (rolling default): transfer fee 0.01 per mille of the amount in both directions on all markets (since 2022-04-29), and stamp duty 0.5 per mille on sells (since 2023-08-28).

    :param float commission: the commission ratio
    :param float lowest_commission: the lowest commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee
    :return: a subclass instance of :py:class:`TradeCostBase`)");

    m.def(
      "TC_Fixed", TC_Fixed, py::arg("commission") = 0.0, py::arg("lowest_commission") = 0.0,
      py::arg("stamptax") = 0.0, py::arg("per_lot") = 0.0, py::arg("others") = 0.0,
      R"(TC_Fixed([commission=0.0, lowest_commission=0.0, stamptax=0.0, per_lot=0.0, others=0.0])

    The generic fixed-rate trade cost algorithm for the futures, FX and crypto markets: the fee is max(commission * price * number * unit, lowest_commission) + per_lot * number + others, and a sell adds the stamp duty stamptax * price * number * unit.

    :param float commission: the commission ratio of the turnover value
    :param float lowest_commission: the lowest commission value
    :param float stamptax: the stamp duty, charged on sells only
    :param float per_lot: the fixed fee per lot (contract)
    :param float others: the fixed extra charge (delivery / funding fees)
    :return: a subclass instance of :py:class:`TradeCostBase`)");

    m.def(
      "TC_FixedA2015", TC_FixedA2015, py::arg("commission") = 0.0018,
      py::arg("lowest_commission") = 5.0, py::arg("stamptax") = 0.001,
      py::arg("transferfee") = 0.00002,
      R"(TC_FixedA2015([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.00002])

    Since August 1, 2015, the SSE transfer fee is changed to 0.02 per mille of the amount

    :param float commission: the commission ratio
    :param float lowest_commission: the lowest commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee
    :return: a subclass instance of :py:class:`TradeCostBase`)");

    m.def(
      "TC_FixedA2017", TC_FixedA2017, py::arg("commission") = 0.0018,
      py::arg("lowest_commission") = 5.0, py::arg("stamptax") = 0.001,
      py::arg("transferfee") = 0.00002,
      R"(TC_FixedA2017([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.00002])

    Since January 1, 2017, the SZSE transfer fee item is listed separately, charged in both directions at 0.02‰ of the amount.

    :param float commission: the commission ratio
    :param float lowest_commission: the lowest commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee
    :return: a subclass instance of :py:class:`TradeCostBase`)");

    m.def(
      "TC_FixedAPre2015", TC_FixedAPre2015, py::arg("commission") = 0.0018,
      py::arg("lowest_commission") = 5.0, py::arg("stamptax") = 0.001,
      py::arg("transferfee") = 0.001, py::arg("lowest_transferfee") = 1.0,
      R"(TC_FixedAPre2015([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.001, lowest_transferfee=1.0])

    The A-share trade cost algorithm before August 1, 2015; the Shanghai transfer fee is charged by traded quantity (1 yuan per 1000 shares, minimum 1 yuan), and the Shenzhen market charges no transfer fee.

    :param float commission: the commission ratio
    :param float lowest_commission: the lowest commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee (per share)
    :param float lowest_transferfee: the lowest transfer fee
    :return: a subclass instance of :py:class:`TradeCostBase`)");

    m.def("TC_FixedETF", TC_FixedETF, py::arg("commission") = 0.0001,
          py::arg("lowest_commission") = 5.0,
          R"(TC_FixedETF([commission=0.0001, lowest_commission=5.0])

    The ETF trade cost algorithm; the commission is charged in both the buy and the sell directions, with no stamp tax and no transfer fee.

    :param float commission: the commission ratio, defaulting to 1 per ten thousand
    :param float lowest_commission: the lowest commission value, defaulting to 5 yuan per trade
    :return: a subclass instance of :py:class:`TradeCostBase`)");

    m.def("TC_Zero", TC_Zero, "The zero trade cost algorithm");

    m.def("MG_None", MG_None,
          R"(MG_None()

    The plain cash margin model: no leverage, an opening consumes the full nominal value

    :return: a subclass instance of :py:class:`MarginBase`)");

    m.def("MG_Fixed", MG_Fixed, py::arg("rate") = 0.6, py::arg("rate_short") = 0.0,
          py::arg("maintain_rate") = 0.0, py::arg("margin_per_lot") = 0.0,
          R"(MG_Fixed([rate=0.6, rate_short=0.0, maintain_rate=0.0, margin_per_lot=0.0])

    The fixed margin model applied to every security of the account

    :param float rate: the opening margin ratio, in (0, 1]
    :param float rate_short: the short opening ratio; 0 means following rate
    :param float maintain_rate: the maintain margin ratio; 0 means the same as the opening margin
    :param float margin_per_lot: the margin of one lot; 0 means using the rate mode
    :return: a subclass instance of :py:class:`MarginBase`)");

    m.def("MG_ByType", MG_ByType, py::arg("rates") = "", py::arg("maintain_rate") = 0.0,
          py::arg("default_rate") = 0.6,
          R"(MG_ByType([rates="", maintain_rate=0.0, default_rate=0.6])

    The margin model keyed by the security type or the market, for a mixed account

    :param str rates: the "key:ratio" list, e.g. "12:0.10, SHF:0.12"; an all-digit key is a
        security type, any other key is a market code
    :param float maintain_rate: the maintain margin ratio; 0 means the same as the opening margin
    :param float default_rate: the ratio of a security matched by no key, in (0, 1]
    :return: a subclass instance of :py:class:`MarginBase`)");
}
