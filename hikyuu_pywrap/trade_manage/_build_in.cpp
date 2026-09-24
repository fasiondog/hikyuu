/*
 * _build_in.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/build_in.h>
#include "../pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_build_in(py::module& m) {
    m.def(
      "crtTM", &crtTM, py::arg("date") = Datetime(199001010000LL), py::arg("init_cash") = 100000,
      py::arg("cost_func") = TC_Zero(), py::arg("name") = "SYS",
      R"(crtTM([date = Datetime(199001010000), init_cash = 100000, cost_func = TC_Zero(), name = "SYS"])

    Create the trade manager module, managing the trade records and the fund usage of the account
    
    :param Datetime date:  the account establishment date
    :param float init_cash:    the initial capital
    :param TradeCost cost_func: the trade cost algorithm
    :param string name:        the account name
    :rtype: TradeManager)");

    m.def("TC_TestStub", TC_TestStub, "For testing only");

    m.def(
      "TC_FixedA", TC_FixedA, py::arg("commission") = 0.0018, py::arg("lowest_commission") = 5.0,
      py::arg("stamptax") = 0.001, py::arg("transferfee") = 0.001,
      py::arg("lowest_transferfee") = 1.0,
      R"(TC_FixedA([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.001, lowest_transferfee=1.0])

    The A-share trade cost algorithm before August 1, 2015

    :param float commission: the commission ratio
    :param float lowest_commission: the lowest commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee
    :param float lowest_transferfee: the lowest transfer fee
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

    m.def("TC_FixedETF", TC_FixedETF, py::arg("commission") = 0.0001,
          py::arg("lowest_commission") = 5.0,
          R"(TC_FixedETF([commission=0.0001, lowest_commission=5.0])

    The ETF trade cost algorithm; the commission is charged in both the buy and the sell directions, with no stamp tax and no transfer fee.

    :param float commission: the commission ratio, defaulting to 1 per ten thousand
    :param float lowest_commission: the lowest commission value, defaulting to 5 yuan per trade
    :return: a subclass instance of :py:class:`TradeCostBase`)");

    m.def("TC_Zero", TC_Zero, "The zero trade cost algorithm");
}