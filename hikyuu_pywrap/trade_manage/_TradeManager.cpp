/*
 * _TradeManager.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/build_in.h>
#include <hikyuu/trade_manage/Performance.h>
#include <optional>
#include "../gil_policy.h"
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyTradeManagerBase : public TradeManagerBase {
    PY_CLONE(PyTradeManagerBase, TradeManagerBase)

public:
    PyTradeManagerBase() : TradeManagerBase() {
        m_is_python_object = true;
    }

    PyTradeManagerBase(const string& name) : TradeManagerBase(name) {
        m_is_python_object = true;
    }

    PyTradeManagerBase(const string& name, const TradeCostPtr& costFunc)
    : TradeManagerBase(name, costFunc) {
        m_is_python_object = true;
    }

    void _reset() override {
        PYBIND11_OVERLOAD(void, TradeManagerBase, _reset, );
    }

    void updateWithWeight(const Datetime& datetime) override {
        PYBIND11_OVERRIDE_NAME(void, TradeManagerBase, "update_with_weight", updateWithWeight,
                               datetime);
    }

    double getMarginRate(const Datetime& datetime, const Stock& stock) override {
        PYBIND11_OVERRIDE_NAME(double, TradeManagerBase, "get_margin_rate", getMarginRate, datetime,
                               stock);
    }

    price_t initCash() const override {
        PYBIND11_OVERRIDE_NAME(price_t, TradeManagerBase, "init_cash", initCash);
    }

    Datetime initDatetime() const override {
        PYBIND11_OVERRIDE_NAME(Datetime, TradeManagerBase, "init_datetime", initDatetime);
    }

    Datetime firstDatetime() const override {
        PYBIND11_OVERRIDE_NAME(Datetime, TradeManagerBase, "first_datetime", firstDatetime);
    }

    Datetime lastDatetime() const override {
        PYBIND11_OVERRIDE_NAME(Datetime, TradeManagerBase, "last_datetime", lastDatetime);
    }

    price_t currentCash() const override {
        PYBIND11_OVERRIDE_NAME(price_t, TradeManagerBase, "current_cash", currentCash);
    }

    price_t cash(const Datetime& datetime, KQuery::KType ktype) override {
        PYBIND11_OVERLOAD(price_t, TradeManagerBase, cash, datetime, ktype);
    }

    bool have(const Stock& stock) const override {
        PYBIND11_OVERLOAD(bool, TradeManagerBase, have, stock);
    }

    bool haveShort(const Stock& stock) const override {
        PYBIND11_OVERRIDE_NAME(bool, TradeManagerBase, "have_short", haveShort, stock);
    }

    size_t getStockNumber() const override {
        PYBIND11_OVERRIDE_NAME(size_t, TradeManagerBase, "get_stock_num", getStockNumber, );
    }

    size_t getShortStockNumber() const override {
        PYBIND11_OVERRIDE_NAME(size_t, TradeManagerBase, "get_short_stock_num",
                               getShortStockNumber, );
    }

    double getHoldNumber(const Datetime& datetime, const Stock& stock) override {
        PYBIND11_OVERRIDE_NAME(double, TradeManagerBase, "get_hold_num", getHoldNumber, datetime,
                               stock);
    }

    double getShortHoldNumber(const Datetime& datetime, const Stock& stock) override {
        PYBIND11_OVERRIDE_NAME(double, TradeManagerBase, "get_short_hold_num", getShortHoldNumber,
                               datetime, stock);
    }

    double getDebtNumber(const Datetime& datetime, const Stock& stock) override {
        PYBIND11_OVERRIDE_NAME(double, TradeManagerBase, "get_debt_number", getDebtNumber, datetime,
                               stock);
    }

    price_t getDebtCash(const Datetime& datetime) override {
        PYBIND11_OVERRIDE_NAME(price_t, TradeManagerBase, "get_debt_cash", getDebtCash, datetime);
    }

    TradeRecordList getTradeList() const override {
        PYBIND11_OVERRIDE_NAME(TradeRecordList, TradeManagerBase, "get_trade_list", getTradeList, );
    }

    TradeRecordList getTradeList(const Datetime& start, const Datetime& end) const override {
        PYBIND11_OVERRIDE_NAME(TradeRecordList, TradeManagerBase, "get_trade_list", getTradeList,
                               start, end);
    }

    PositionRecordList getPositionList() const override {
        PYBIND11_OVERRIDE_NAME(PositionRecordList, TradeManagerBase, "get_position_list",
                               getPositionList, );
    }

    PositionRecordList getHistoryPositionList() const override {
        PYBIND11_OVERRIDE_NAME(PositionRecordList, TradeManagerBase, "get_history_position_list",
                               getHistoryPositionList, );
    }

    PositionRecordList getShortPositionList() const override {
        PYBIND11_OVERRIDE_NAME(PositionRecordList, TradeManagerBase, "get_short_position_list",
                               getShortPositionList, );
    }

    PositionRecordList getShortHistoryPositionList() const override {
        PYBIND11_OVERRIDE_NAME(PositionRecordList, TradeManagerBase,
                               "get_short_history_position_list", getShortHistoryPositionList, );
    }

    PositionRecord getPosition(const Datetime& date, const Stock& stock) override {
        PYBIND11_OVERRIDE_NAME(PositionRecord, TradeManagerBase, "get_position", getPosition, date,
                               stock);
    }

    PositionRecord getShortPosition(const Stock& stock) const override {
        PYBIND11_OVERRIDE_NAME(PositionRecord, TradeManagerBase, "get_short_position",
                               getShortPosition, stock);
    }

    BorrowRecordList getBorrowStockList() const override {
        PYBIND11_OVERRIDE_NAME(BorrowRecordList, TradeManagerBase, "get_borrow_stock_list",
                               getBorrowStockList, );
    }

    bool checkin(const Datetime& datetime, price_t cash) override {
        PYBIND11_OVERLOAD(bool, TradeManagerBase, checkin, datetime, cash);
    }

    bool checkout(const Datetime& datetime, price_t cash) override {
        PYBIND11_OVERLOAD(bool, TradeManagerBase, checkout, datetime, cash);
    }

    bool checkinStock(const Datetime& datetime, const Stock& stock, price_t price,
                      double number) override {
        PYBIND11_OVERRIDE_NAME(bool, TradeManagerBase, "checkin_stock", checkinStock, datetime,
                               stock, price, number);
    }

    bool checkoutStock(const Datetime& datetime, const Stock& stock, price_t price,
                       double number) override {
        PYBIND11_OVERRIDE_NAME(bool, TradeManagerBase, "checkout_stock", checkoutStock, datetime,
                               stock, price, number);
    }

    TradeRecord buy(const Datetime& datetime, const Stock& stock, price_t realPrice, double number,
                    price_t stoploss, price_t goalPrice, price_t planPrice, SystemPart from,
                    const string& remark) override {
        PYBIND11_OVERLOAD(TradeRecord, TradeManagerBase, buy, datetime, stock, realPrice, number,
                          stoploss, goalPrice, planPrice, from, remark);
    }

    TradeRecord sell(const Datetime& datetime, const Stock& stock, price_t realPrice, double number,
                     price_t stoploss, price_t goalPrice, price_t planPrice, SystemPart from,
                     const string& remark) override {
        PYBIND11_OVERLOAD(TradeRecord, TradeManagerBase, sell, datetime, stock, realPrice, number,
                          stoploss, goalPrice, planPrice, from, remark);
    }

    TradeRecord sellShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                          double number, price_t stoploss, price_t goalPrice, price_t planPrice,
                          SystemPart from, const string& remark) override {
        PYBIND11_OVERRIDE_NAME(TradeRecord, TradeManagerBase, "sell_short", sellShort, datetime,
                               stock, realPrice, number, stoploss, goalPrice, planPrice, from,
                               remark);
    }

    TradeRecord buyShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                         double number, price_t stoploss, price_t goalPrice, price_t planPrice,
                         SystemPart from, const string& remark) override {
        PYBIND11_OVERRIDE_NAME(TradeRecord, TradeManagerBase, "buy_short", buyShort, datetime,
                               stock, realPrice, number, stoploss, goalPrice, planPrice, from,
                               remark);
    }

    bool borrowCash(const Datetime& datetime, price_t cash) override {
        PYBIND11_OVERRIDE_NAME(bool, TradeManagerBase, "borrow_cash", borrowCash, datetime, cash);
    }

    bool returnCash(const Datetime& datetime, price_t cash) override {
        PYBIND11_OVERRIDE_NAME(bool, TradeManagerBase, "return_cash", returnCash, datetime, cash);
    }

    bool borrowStock(const Datetime& datetime, const Stock& stock, price_t price,
                     double number) override {
        PYBIND11_OVERRIDE_NAME(bool, TradeManagerBase, "borrow_stock", borrowStock, datetime, stock,
                               price, number);
    }

    bool returnStock(const Datetime& datetime, const Stock& stock, price_t price,
                     double number) override {
        PYBIND11_OVERRIDE_NAME(bool, TradeManagerBase, "return_stock", returnStock, datetime, stock,
                               price, number);
    }

    FundsRecord getFunds(KQuery::KType ktype) const override {
        PYBIND11_OVERRIDE_NAME(FundsRecord, TradeManagerBase, "get_funds", getFunds, ktype);
    }

    FundsRecord getFunds(const Datetime& datetime, KQuery::KType ktype) override {
        PYBIND11_OVERRIDE_NAME(FundsRecord, TradeManagerBase, "get_funds", getFunds, datetime,
                               ktype);
    }

    bool addTradeRecord(const TradeRecord& tr) override {
        PYBIND11_OVERRIDE_NAME(bool, TradeManagerBase, "add_trade_record", addTradeRecord, tr);
    }

    bool addPosition(const PositionRecord& pr) override {
        PYBIND11_OVERRIDE_NAME(bool, TradeManagerBase, "add_position", addPosition, pr);
    }

    string str() const override {
        PYBIND11_OVERRIDE_NAME(string, TradeManagerBase, "__str__", str, );
    }

    void tocsv(const string& path) override {
        PYBIND11_OVERLOAD(void, TradeManagerBase, tocsv, path);
    }

    void fetchAssetInfoFromBroker(const OrderBrokerPtr& broker, const Datetime& datetime) override {
        PYBIND11_OVERRIDE_NAME(void, TradeManagerBase, "fetch_asset_info_from_broker",
                               fetchAssetInfoFromBroker, broker, datetime);
    }
};

TradeCostPtr (TradeManagerBase::*get_costFunc)() const = &TradeManagerBase::costFunc;
void (TradeManagerBase::*set_costFunc)(const TradeCostPtr&) = &TradeManagerBase::costFunc;

MarginPtr (TradeManagerBase::*get_margin)() const = &TradeManagerBase::margin;
void (TradeManagerBase::*set_margin)(const MarginPtr&) = &TradeManagerBase::margin;

TradeRecordList (TradeManagerBase::*_getTradeList_1)() const = &TradeManagerBase::getTradeList;
TradeRecordList (TradeManagerBase::*_getTradeList_2)(const Datetime&, const Datetime&) const =
  &TradeManagerBase::getTradeList;

void export_TradeManager(py::module& m) {
    py::class_<TradeManagerBase, TradeManagerPtr, PyTradeManagerBase>(
      m, "TradeManager", py::dynamic_attr(),
      R"(The trade manager class, which can be understood as a simulated account performing the simulated trades. Generally, use crtTM to create the trade manager instance.

The trade management can be understood as a simulated account performing the simulated trades. Generally, use crtTM to create the trade manager instance.

Common parameters:

    - precision=2 (int) : the price calculation accuracy
    - support_borrow_cash=False (bool) : whether to finance automatically
    - support_borrow_stock=False (bool) : whether to short the securities automatically
    - save_action=True (bool) : whether to save the Python command sequences
    - margin_accounting=False (bool) : whether to book the margin securities by margin
    - settle_with_close=True (bool) : whether the daily settlement posts at the close of the K type
    - force_liquidate=False (bool) : whether to queue the forced liquidation on a margin call
    - annual_days=365 (int) : the days of a year, the denominator prorating the margin interest)")
      .def(py::init<>())
      .def(py::init<const string&, const TradeCostPtr&>())

      .def("__str__", &TradeManagerBase::str)
      .def("__repr__", &TradeManagerBase::str)

      .def_property("name", py::overload_cast<>(&TradeManagerBase::name, py::const_),
                    py::overload_cast<const string&>(&TradeManagerBase::name),
                    py::return_value_policy::copy, "Name")

      .def_property_readonly("init_cash", &TradeManagerBase::initCash,
                             "(Read-only) the initial capital")
      .def_property_readonly("current_cash", &TradeManagerBase::currentCash,
                             "(Read-only) the current cash")
      .def_property_readonly("init_datetime", &TradeManagerBase::initDatetime,
                             "(Read-only) the account establishment date")

      .def_property_readonly("first_datetime", &TradeManagerBase::firstDatetime,
                             "(Read-only) the date when the first buy trade occurred; if no trade "
                             "has occurred, return Null<Datetime>()")
      .def_property_readonly(
        "last_datetime", &TradeManagerBase::lastDatetime,
        "(Read-only) the date of the last trade; note that it is unrelated to the trade type; if "
        "no trade has occurred, return the account establishment date")
      .def_property_readonly(
        "precision", &TradeManagerBase::precision,
        "(Read-only) the price precision, the same as the common parameter 'precision'")

      .def_property("cost_func", get_costFunc, set_costFunc, "The trade cost algorithm")

      .def_property("margin", get_margin, py::cpp_function(set_margin, py::keep_alive<1, 2>()),
                    R"(The margin model of the account (MarginBase)

    Setting it to None falls back to the plain cash model MG_None. The default model of a new
    account is MG_Fixed(0.6), i.e. the legacy 60 % buying power.)")

      .def("get_open_margin", &TradeManagerBase::getOpenMargin, py::arg("date"), py::arg("stock"),
           py::arg("price"), py::arg("num"), py::arg("side") = MarginSide::LONG,
           R"(get_open_margin(self, date, stock, price, num, side=MarginSide.LONG)

    Get the margin occupied by an opening, according to the account margin model

    :param Datetime date: the trade moment
    :param Stock stock: the security to open
    :param float price: the opening price
    :param float num: the opening quantity
    :param MarginSide side: the long or the short side
    :return: the occupied funds
    :rtype: float)")

      .def("get_maintain_margin", &TradeManagerBase::getMaintainMargin, py::arg("date"),
           py::arg("stock"), py::arg("price"), py::arg("num"), py::arg("side") = MarginSide::LONG,
           R"(get_maintain_margin(self, date, stock, price, num, side=MarginSide.LONG)

    Get the maintain margin (the additional-margin threshold) of an opening

    :return: the maintain margin
    :rtype: float)")

      .def("get_max_open_number", &TradeManagerBase::getMaxOpenNumber, py::arg("date"),
           py::arg("stock"), py::arg("price"), py::arg("available"),
           py::arg("side") = MarginSide::LONG,
           R"(get_max_open_number(self, date, stock, price, available, side=MarginSide.LONG)

    Get the maximum openable quantity under the given available funds

    :param float available: the available funds
    :return: the quantity, 0 when nothing can be opened
    :rtype: float)")

      .def("get_occupied_margin", &TradeManagerBase::getOccupiedMargin, py::arg("date"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_occupied_margin(self, date, ktype="DAY")

    The margin occupied by the current long and short positions at the given moment

    :param Datetime date: the given moment
    :param str ktype: the K-line type used to get the position price
    :return: the occupied margin
    :rtype: float)")

      .def("get_available_margin", &TradeManagerBase::getAvailableMargin, py::arg("date"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_available_margin(self, date, ktype="DAY")

    The available funds for a new opening, i.e. the net assets minus the collateral haircut and
    the occupied margin

    :param Datetime date: the given moment
    :param str ktype: the K-line type used to get the position price
    :return: the available funds
    :rtype: float)")

      .def("get_collateral_rate", &TradeManagerBase::getCollateralRate, py::arg("date"),
           py::arg("stock"), py::arg("side") = MarginSide::LONG,
           R"(get_collateral_rate(self, date, stock, side=MarginSide.LONG)

    Collateral conversion rate of the security, answered by the account margin model

    :param Datetime date: the queried moment
    :param Stock stock: the held security
    :param MarginSide side: the long or the short side
    :return: the part of the holding counted as collateral, 1.0 without a margin model
    :rtype: float)")

      .def("get_collateral_haircut", &TradeManagerBase::getCollateralHaircut, py::arg("date"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_collateral_haircut(self, date, ktype="DAY")

    The value discounted away by the collateral table, i.e. Σ collateral base × (1 - conversion
    rate) over the long holdings. A margin-accounted holding bases the discount on what it
    contributes to the net assets (the locked margin plus the unrealized profit) instead of on its
    full market value, so the financed part is not taken away twice

    :param Datetime date: the given moment
    :param str ktype: the K-line type used to get the position price
    :return: the haircut, 0 when no conversion rate is configured
    :rtype: float)")

      .def("is_margin_accounted", &TradeManagerBase::isMarginAccounted, py::arg("stock"),
           R"(is_margin_accounted(self, stock)

    Whether an opening of the stock is booked by the margin accounting instead of the full cash

    :param Stock stock: the security to check
    :return: True when the opening locks the margin only
    :rtype: bool)")

      .def("settlement", &TradeManagerBase::settlement, py::arg("date"),
           py::arg("ktype") = KQuery::DAY,
           R"(settlement(self, date, ktype="DAY")

    Settle the margin holdings as of the given moment

    Every margin-accounted holding is marked to the settlement price (the close of the given
    K type), the mark-to-market profit is posted into the cash, the locked margin is recomputed at
    that price with the difference posted, and the maintenance requirement is then checked against
    the equity. A shortfall only warns unless force_liquidate is on, which queues the flattening
    for the trading system to execute at the next open. Before that check the day interest of the
    outstanding liabilities is billed into the debt itself (0.0 rates bill nothing). Both halves
    are gated per calendar day, so settling twice on one day posts nothing again.

    :param Datetime date: the settlement moment, later than or equal to lastDatetime()
    :param str ktype: the K-line type the settlement price is taken from
    :return: True when the calendar day of the moment ends up settled, False when the account never
             settles
    :rtype: bool)")

      .def("is_settled", &TradeManagerBase::isSettled, py::arg("date"),
           R"(is_settled(self, date)

    Whether the calendar day of the given moment has entered a settlement batch. The batch has two
    independently gated halves, the posting and the billing, so True does not by itself say that
    the holdings were marked on that day

    :param Datetime date: the moment to probe
    :rtype: bool)")

      .def("is_forced_liquidation_queued", &TradeManagerBase::isForcedLiquidationQueued,
           py::arg("date"),
           R"(is_forced_liquidation_queued(self, date)

    Whether a forced liquidation is queued as of the given moment

    :param Datetime date: the moment to probe
    :rtype: bool)")

      .def_property("broker_last_datetime", &TradeManagerBase::getBrokerLastDatetime,
                    &TradeManagerBase::setBrokerLastDatetime,
                    R"(The moment when the order broker operations actually start.

    By default, when the TradeManager executes the buy/sell operations, it calls the order broker to execute the broker's buy/sell actions, but there will be a problem in the live trading operation. Because the system needs to backtrack the historical data to get the latest signal when calculating the signal generator, the TradeManager will execute the buy/sell operations at the historical moments; at this time, if the order broker itself does not control the moment of issuing the buy/sell instructions, it will cause the broker to send the wrong instructions. At this time, it is necessary to specify that only after a certain moment are the buy/sell operations of the order broker allowed to be specified. The attribute brokeLastDatetime is used to specify that moment.))")

      .def("get_param", &TradeManagerBase::getParam<boost::any>, R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (TradeManagerBase::*)(const std::string&, const boost::any&)>(
             &TradeManagerBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :type value: int | bool | float | string | Query | KData | Stock | DatetimeList
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &TradeManagerBase::haveParam, "Whether the specified parameter exists")

      .def("reset", &TradeManagerBase::reset, "Reset, clearing the trade and the position records")
      .def("clone", &TradeManagerBase::clone, "Clone (deep copy) the instance")

      .def("reg_broker", &TradeManagerBase::regBroker, R"(reg_broker(self, broker)

    Register the order broker. This command can be executed multiple times to register multiple order brokers.

    :param OrderBrokerBase broker: the order broker instance)")

      .def("clear_broker", &TradeManagerBase::clearBroker, R"(clear_broker(self)

    Clear all the registered order brokers)")

      .def("get_margin_rate", &TradeManagerBase::getMarginRate)

      .def("have", &TradeManagerBase::have, R"(have(self, stock)

    Whether the specified security is currently held

    :param Stock stock: the specified security
    :rtype: bool)")

      .def("have_short", &TradeManagerBase::haveShort, R"(have_short(self, stock)

    Whether the given security is currently held in the short position

    :param Stock stock: the specified security
    :rtype: bool)")

      .def("get_stock_num", &TradeManagerBase::getStockNumber, R"(get_stock_num(self)

    The number of the kinds of the securities currently held, i.e. how many stocks are currently held (not the position size of each stock)

    :rtype: int)")

      .def("get_short_stock_num", &TradeManagerBase::getShortStockNumber)

      .def("get_hold_num", &TradeManagerBase::getHoldNumber, R"(get_hold_num(self, datetime, stock)

        Get the holding quantity of the specified security at the specified moment

        :param Datetime datetime: the specified moment
        :param Stock stock: the specified security
        :rtype: int)")

      .def("get_short_hold_num", &TradeManagerBase::getShortHoldNumber)

      .def("get_debt_number", &TradeManagerBase::getDebtNumber,
           R"(get_debt_number(self, datetime, stock)

    Get the number of the borrowed shares at the specified moment

    :param Datetime datetime: the specified moment
    :param Stock stock: the specified security
    :rtype: float)")

      .def("get_debt_cash", &TradeManagerBase::getDebtCash, R"(get_debt_cash(self, datetime)

    Get the borrowed cash amount at the specified moment

    :param Datetime datetime: the specified moment
    :rtype: float)")

      .def("get_trade_list", _getTradeList_1)
      .def("get_trade_list", _getTradeList_2, R"(get_trade_list(self[, start, end])

    Get the trade records; when the parameters are not specified, get all the trade records

    :param Datetime start: the start date
    :param Datetime end: the end date
    :rtype: TradeRecordList)")

      .def("get_position_list", &TradeManagerBase::getPositionList, R"(get_position_list(self)

    Get all the current position records

    :rtype: PositionRecordList)")

      .def("get_positions", &TradeManagerBase::getPositionDict, R"(get_positions(self)

    Get all the current position records as a dictionary, with the stock as the key and the PositionRecord as the value

    :rtype: dict)")

      .def("get_history_position_list", &TradeManagerBase::getHistoryPositionList,
           R"(get_history_position_list(self)

    Get all the historical position records, i.e. the closed records

    :rtype: PositionRecordList)")

      .def("get_short_position_list", &TradeManagerBase::getShortPositionList,
           R"(get_short_position_list(self)

    Get all the current short position records

    :rtype: PositionRecordList)")

      .def("get_short_history_position_list", &TradeManagerBase::getShortHistoryPositionList,
           R"(get_short_history_position_list(self)

    Get all the historical short position records, i.e. the closed records

    :rtype: PositionRecordList)")

      .def("get_short_position", &TradeManagerBase::getShortPosition,
           R"(get_short_position(self, stock)

    Get the current short position record; if not held short, return PositionRecord()

    :param Stock stock: the specified security
    :rtype: PositionRecord)")

      .def("get_borrow_stock_list", &TradeManagerBase::getBorrowStockList,
           R"(get_borrow_stock_list(self)

    Get the list of the currently borrowed shares

    :rtype: BorrowRecordList)")

      .def("get_position", &TradeManagerBase::getPosition, R"(get_position(self, date, stock)

    Get the position record of the security on the specified date; if the stock is not currently held, return PositionRecord()
    Note: only number is the exact value on the specified date; the other economic fields (e.g. takeDatetime, buyMoney, totalCost,
    sellMoney) are taken from the most recent closed position record of the security, for reference only

    :param Datetime date: the specified date
    :param Stock stock: the specified security
    :rtype: PositionRecord)")

      .def("get_buy_cost", &TradeManagerBase::getBuyCost,
           R"(get_buy_cost(self, datetime, stock, price, num)

    Calculate the buy cost

    :param Datetime datetime: the trading time
    :param Stock stock:       the security traded
    :param float price:       the buy price
    :param float num:         the buy quantity
    :rtype: CostRecord)")

      .def("get_sell_cost", &TradeManagerBase::getSellCost,
           R"(get_sell_cost(self, datetime, stock, price, num)

    Calculate the sell cost

    :param Datetime datetime: the trading time
    :param Stock stock:       the security traded
    :param float price:       the sell price
    :param float num:         the sell quantity
    :rtype: CostRecord)")

      .def("get_borrow_cash_cost", &TradeManagerBase::getBorrowCashCost)
      .def("get_return_cash_cost", &TradeManagerBase::getReturnCashCost)
      .def("get_borrow_stock_cost", &TradeManagerBase::getBorrowStockCost)
      .def("get_return_stock_cost", &TradeManagerBase::getReturnStockCost)
      .def("get_borrow_cash_rate", &TradeManagerBase::getBorrowCashRate, py::arg("date"),
           R"(get_borrow_cash_rate(self, date)

    Get the annualized interest rate of the borrowed cash, answered by the cost model. It is billed
    by the daily settlement, prorated by the annual_days parameter of the account.

    :param Datetime date: the settled moment
    :return: an annualized decimal rate, 0.0 bills nothing
    :rtype: float)")

      .def("get_borrow_stock_rate", &TradeManagerBase::getBorrowStockRate, py::arg("date"),
           py::arg("stock"),
           R"(get_borrow_stock_rate(self, date, stock)

    Get the annualized interest rate of a borrowed security, answered by the cost model

    :param Datetime date: the settled moment
    :param Stock stock:   the borrowed security
    :return: an annualized decimal rate, 0.0 bills nothing
    :rtype: float)")

      .def("cash", &TradeManagerBase::cash, py::arg("datetime"), py::arg("ktype") = KQuery::DAY,
           R"(cash(self, datetime[, ktype=Query.KType.DAY])

    Get the cash at the specified moment. (Note: without the date parameter, the positions cannot be adjusted according to the dividend information.)

    :param Datetime datetime: the specified moment
    :param ktype: the K-line type
    :rtype: float)")

      .def(
        "get_funds",
        [](TradeManagerBase& self, const KQuery::KType& ktype) {
            ScopeGilRelease gil(!self.hasPythonPart());
            return self.getFunds(ktype);
        },
        py::arg("ktype") = KQuery::DAY)
      .def(
        "get_funds",
        [](TradeManagerBase& self, const Datetime& datetime, const KQuery::KType& ktype) {
            ScopeGilRelease gil(!self.hasPythonPart());
            return self.getFunds(datetime, ktype);
        },
        py::arg("datetime"), py::arg("ktype") = KQuery::DAY,
        R"(get_funds(self, [datetime, ktype = Query.DAY])

    Get the asset market value details at the specified moment

    :param Datetime datetime:  the specified moment
    :param Query.KType ktype: the K-line type
    :rtype: FundsRecord)")

      .def(
        "get_funds_list",
        [](TradeManagerBase& self, const DatetimeList& dates, const KQuery::KType& ktype) {
            ScopeGilRelease gil(!self.hasPythonPart());
            return self.getFundsList(dates, ktype);
        },
        py::arg("dates"), py::arg("ktype") = KQuery::DAY,
        R"(get_funds_list(self, dates[, ktype = Query.DAY])

    Get the daily asset records of the specified date list
    :param DatetimeList dates:  the specified moments
    :param Query.KType ktype: the K-line type
    :rtype: FundsList)")

      .def(
        "get_funds_curve",
        [](TradeManagerBase& self, const DatetimeList& dates, const KQuery::KType& ktype) {
            ScopeGilRelease gil(!self.hasPythonPart());
            return self.getFundsCurve(dates, ktype);
        },
        py::arg("dates"), py::arg("ktype") = KQuery::DAY,
        R"(get_funds_curve(self, dates[, ktype = Query.DAY])

    Get the net asset value curve

    :param DatetimeList dates: the date list; get the corresponding net asset value curve according to this date list
    :param Query.KType ktype: the K-line type, which must match the date list
    :return: the net asset value list
    :rtype: PriceList)")

      .def("get_profit_curve", &TradeManagerBase::getProfitCurve, py::arg("dates"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_profit_curve(self, dates[, ktype = Query.DAY])

    Get the profit curve, i.e. the net asset value curve after deducting the previous deposits

    :param DatetimeList dates: the date list; get the corresponding profit curve according to this date list, which should be in the increasing order
    :param Query.KType ktype: the K-line type, which must match the date list
    :return: the profit curve
    :rtype: PriceList)")

      .def("get_profit_cum_change_curve", &TradeManagerBase::getProfitCumChangeCurve,
           py::arg("dates"), py::arg("ktype") = KQuery::DAY,
           R"(get_profit_cum_change_curve(self, dates[, ktype = Query.DAY])

    Get the cumulative return curve, i.e. total assets divided by the invested base, rounded by
    the account precision; when the invested base is 0 the ratio is undefined and Null (NaN)
    is returned

    :param DatetimeList dates: the date list
    :param Query.KType ktype: the K-line type, which must match the date list
    :rtype: PriceList)")

      .def("get_base_assets_curve", &TradeManagerBase::getBaseAssetsCurve, py::arg("dates"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_base_assets_curve(self, dates[, ktype = Query.DAY])

    Get the invested principal asset curve (the invested capital)

    :param DatetimeList dates: the date list
    :param Query.KType ktype: the K-line type, which must match the date list
    :rtype: PriceList)")

      .def("checkin", &TradeManagerBase::checkin, R"(checkin(self, datetime, cash)

    Deposit the cash into the account

    :param Datetime datetime: the trading time
    :param float cash: the amount of the cash deposited
    :rtype: bool)")

      .def("checkout", &TradeManagerBase::checkout, R"(checkout(self, datetime, cash)

    Withdraw the cash from the account

    :param Datetime datetime: the trading time
    :param float cash: the amount of the funds withdrawn
    :rtype: bool)")

      .def("checkin_stock", &TradeManagerBase::checkinStock)
      .def("checkout_stock", &TradeManagerBase::checkoutStock)
      .def("borrow_cash", &TradeManagerBase::borrowCash)
      .def("return_cash", &TradeManagerBase::returnCash)
      .def("borrow_stock", &TradeManagerBase::borrowStock)
      .def("return_stock", &TradeManagerBase::returnStock)

      .def(
        "buy", &TradeManagerBase::buy, py::arg("datetime"), py::arg("stock"), py::arg("real_price"),
        py::arg("num"), py::arg("stoploss") = 0.0, py::arg("goal_price") = 0.0,
        py::arg("plan_price") = 0.0, py::arg("part") = PART_INVALID, py::arg("remark") = "",
        R"(buy(self, datetime, stock, real_price, number[, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])

    The buy operation

    :param Datetime datetime: the buy time
    :param Stock stock:       the security to buy
    :param float real_price:  the actual buy price; 0 is a legal market-order placeholder
        (the dealing price is decided by the broker)
    :param float num:         the buy quantity
    :param float stoploss:    the stop-loss price
    :param float goal_price:  the target price
    :param float plan_price:  the planned buy price
    :param SystemPart part:   the source of the trading instruction
    :param string remark:     the remark information
    :rtype: TradeRecord)")

      .def(
        "sell", &TradeManagerBase::sell, py::arg("datetime"), py::arg("stock"),
        py::arg("real_price"), py::arg("num") = MAX_DOUBLE, py::arg("stoploss") = 0.0,
        py::arg("goal_price") = 0.0, py::arg("plan_price") = 0.0, py::arg("part") = PART_INVALID,
        py::arg("remark") = "",
        R"(sell(self, datetime, stock, realPrice[, number=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])

    The sell operation

    :param Datetime datetime: the sell time
    :param Stock stock:       the security to sell
    :param float real_price:  the actual sell price; 0 is a legal market-order placeholder
        (the dealing price is decided by the broker)
    :param float num:         the sell quantity; if it equals constant.max_double, it means selling all
    :param float stoploss:    the new stop-loss price
    :param float goal_price:  the new target price
    :param float plan_price:  the originally planned sell price
    :param SystemPart part:   the source of the trading instruction
    :param string remark:     the trade remark
    :rtype: TradeRecord)")

      .def(
        "buy_short", &TradeManagerBase::buyShort, py::arg("datetime"), py::arg("stock"),
        py::arg("real_price"), py::arg("num") = MAX_DOUBLE, py::arg("stoploss") = 0.0,
        py::arg("goal_price") = 0.0, py::arg("plan_price") = 0.0, py::arg("part") = PART_INVALID,
        py::arg("remark") = "",
        R"(buy_short(self, datetime, stock, real_price[, num=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])

    The short-side buy operation, i.e. closing a short position

    :param Datetime datetime: the trade time
    :param Stock stock:       the security traded
    :param float real_price:  the actual trade price
    :param float num:         the trade quantity; constant.max_double means closing all
    :param float stoploss:    the stop-loss price
    :param float goal_price:  the target price
    :param float plan_price:  the originally planned price
    :param SystemPart part:   the source of the trading instruction
    :param string remark:     the trade remark
    :rtype: TradeRecord)")

      .def(
        "sell_short", &TradeManagerBase::sellShort, py::arg("datetime"), py::arg("stock"),
        py::arg("real_price"), py::arg("num"), py::arg("stoploss") = 0.0,
        py::arg("goal_price") = 0.0, py::arg("plan_price") = 0.0, py::arg("part") = PART_INVALID,
        py::arg("remark") = "",
        R"(sell_short(self, datetime, stock, real_price, num[, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID, remark=""])

    The short-side sell operation, i.e. opening a short position

    :param Datetime datetime: the trade time
    :param Stock stock:       the security traded
    :param float real_price:  the actual trade price
    :param float num:         the trade quantity
    :param float stoploss:    the stop-loss price
    :param float goal_price:  the target price
    :param float plan_price:  the originally planned price
    :param SystemPart part:   the source of the trading instruction
    :param string remark:     the trade remark
    :rtype: TradeRecord)")

      .def("add_trade_record", &TradeManagerBase::addTradeRecord, R"(add_trade_record(self, tr)

    Add the trade record directly; if the initialization account record is added, all the existing trade and position records will be cleared.

    :param TradeRecord tr: the trade record
    :return: True (success) | False (failure)
    :rtype: bool)")

      .def("add_position", &TradeManagerBase::addPosition, R"(add_position(self, position)

    After establishing the initial account, add the position record directly; it is only used to build an account with the initial positions

    :param PositionRecord position: the position record
    return True | False)")

      .def("tocsv", &TradeManagerBase::tocsv, R"(tocsv(self, path)

    Output the trade records, the open position records, the closed position records and the net asset value curve in the csv format

    :param str path: the directory of the output files)")

      .def(
        "update_with_weight",
        [](TradeManagerBase& self, const Datetime& date) {
            ScopeGilRelease gil(!self.hasPythonPart());
            return self.updateWithWeight(date);
        },
        R"(update_with_weight(self, date)

      Update the current positions and the trade records according to the equity/dividend adjustment information; it must be called in the chronological order

      :param Datetime date: the current moment)")

      .def("fetch_asset_info_from_broker", &TradeManagerBase::fetchAssetInfoFromBroker,
           py::arg("broker"), py::arg("date") = Datetime(),
           R"(fetch_asset_info_from_broker(self, date)

      Synchronize the asset information at the current moment from the Broker; it must be called in the chronological order

      :param broker the order broker instance
      :param date when synchronizing, it is usually the current time (Null); it can also be forced to a specified time point)")

      .def(
        "get_performance",
        [](TradeManagerBase& self, std::optional<Datetime> datetime, const KQuery::KType& ktype,
           bool ext) {
            ScopeGilRelease gil(!self.hasPythonPart());
            return self.getPerformance(datetime.value_or(Datetime::now()), ktype, ext);
        },
        py::arg("datetime") = py::none(), py::arg("ktype") = KQuery::DAY, py::arg("ext") = true,
        R"(get_performance(self[, datetime=None, ktype=Query.DAY]) -> Performance)

    Get the account performance at the specified moment of the account

    :param Datetime datetime: the specified moment (defaults to the current time)
    :param Query.KType ktype: the K-line type
    :param bool ext: whether to get the extended statistics items (the donating user); otherwise, they are still the basic statistics items
    :return: the account performance)")

      .def(
        "get_max_pull_back",
        [](TradeManagerBase& self, std::optional<Datetime> date, const KQuery::KType& ktype) {
            return self.getMaxPullBack(date.value_or(Datetime::now()), ktype);
        },
        py::arg("date") = py::none(), py::arg("ktype") = KQuery::DAY,
        R"(get_max_pull_back(self, date=None, ktype=Query.DAY) -> float

    Get the maximum drawdown percentage of the account at the specified moment (a negative number)

    :param Datetime date: the specified date (including this moment; defaults to the current time)
    :param Query.KType ktype: the K-line type
    :return: the maximum drawdown percentage)")

      .def(
        "get_position_ext_info", &TradeManagerBase::getPositionExtInfo, py::arg("stock"),
        py::arg("current_time"), py::arg("ktype") = KQuery::DAY, py::arg("trade_mode") = 0,
        R"(get_position_ext_info(self, stock, current_time, ktype=Query.DAY, trade_mode=0) -> PositionExtInfo)

        Get the extended position details at the specified moment of the account (only for the specified stock)

        :param Stock stock: the specified stock
        :param Datetime current_time: the current moment (it needs to be greater than or equal to the last trading moment)
        :param Query.KType ktype: the K-line type, defaulting to the daily line
        :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open, defaulting to 0
        :return: the extended position details, containing the following fields:

            - position (PositionRecord): the basic position record
            - max_high_price (float): the maximum of the highest prices in the period
            - min_low_price (float): the minimum of the lowest prices in the period
            - max_close_price (float): the highest close price in the period
            - min_close_price (float): the lowest close price in the period
            - current_close_price (float): the current close price
            - max_pull_back1 (float): the maximum drawdown percentage 1 (calculated only with the maximum close price and the lowest close price) (a negative number)
            - max_pull_back2 (float): the maximum drawdown percentage 2 (calculated with the maximum of the highest prices and the minimum of the lowest prices in the period) (a negative number)
            - current_profit (float): the current floating profit and loss (excluding the estimated sell cost)

            And the following calculation methods:

            - current_pull_back1(): the current drawdown percentage 1 (calculated only with the maximum close price and the current close price)
            - current_pull_back2(): the current drawdown percentage 2 (calculated with the maximum of the highest prices in the period and the current close price)
            - max_floating_profit1(): the maximum floating profit percentage 1 in the period (calculated only with the close price, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)
            - max_floating_profit2(): the maximum floating profit percentage 2 in the period (calculated with the maximum of the highest prices, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)
            - min_loss_profit1(): the maximum floating loss percentage 1 in the period (calculated only with the close price, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)
            - min_loss_profit2(): the maximum floating loss percentage 2 in the period (calculated only with the lowest price in the period, excluding the estimated sell cost; the statistics are inaccurate when buying and selling multiple times)

        :note: this function is only suitable for the case of one buy and one sell; for the case of one buy and multiple sells, some statistics may be inaccurate, for reference only
        :rtype: PositionExtInfo
        )")

      .def(
        "get_position_ext_info_list", &TradeManagerBase::getPositionExtInfoList,
        py::arg("current_time"), py::arg("ktype") = KQuery::DAY, py::arg("trade_mode") = 0,
        R"(get_position_ext_info_list(self, current_time, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo])

    Get the position details (the open position records) of the specified time after the last trading moment of the account

    :param Datetime current_time: the current moment (it needs to be greater than or equal to the last trading moment)
    :param Query.KType ktype: the K-line type
    :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open
    :return: the list of the extended position details
    :rtype: list[PositionExtInfo])")

      .def(
        "get_position_ext_info_dict", &TradeManagerBase::getPositionExtInfoDict,
        py::arg("current_time"), py::arg("ktype") = KQuery::DAY, py::arg("trade_mode") = 0,
        R"(get_position_ext_info_dict(self, current_time, ktype=Query.DAY, trade_mode=0) -> dict[Stock, PositionExtInfo])

    Get the position details of the specified time after the last trading moment of the account, returned as a dictionary, with the stock as the key and the PositionExtInfo as the value

    :param Datetime current_time: the current moment (it needs to be greater than or equal to the last trading moment)
    :param Query.KType ktype: the K-line type
    :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open
    :return: the dict of the extended position details, with the stock as the key and the PositionExtInfo as the value
    :rtype: dict[Stock, PositionExtInfo])")

      .def(
        "get_history_position_ext_info_list", &TradeManagerBase::getHistoryPositionExtInfoList,
        py::arg("ktype") = KQuery::DAY, py::arg("trade_mode") = 0,
        R"(get_history_position_ext_info_list(self, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo])

    Get the historical position extended details of the account (the closed records); entries
    whose K-line data is empty cannot be computed and are returned with the default (all zero)
    extended fields, i.e. the data is missing

    :param Query.KType ktype: the K-line type
    :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open
    :return: the list of the extended position details
    :rtype: list[PositionExtInfo])")

      .def(
        "get_profit_percent_monthly",
        [](TradeManagerBase& self, std::optional<Datetime> datetime) {
            return self.getProfitPercentMonthly(datetime.value_or(Datetime::now()));
        },
        py::arg("datetime") = py::none(),
        R"(get_profit_percent_monthly(self, datetime=None) -> list[tuple[Datetime, double]]

    Get the account profit percentage (monthly) of the account at the specified deadline,
    excluding the external cash flows (check in / check out)

    :param Datetime datetime: the specified deadline (defaults to the current time)
    :return: the account profit percentage (monthly))")

      .def(
        "get_profit_percent_yearly",
        [](TradeManagerBase& self, std::optional<Datetime> datetime) {
            return self.getProfitPercentYearly(datetime.value_or(Datetime::now()));
        },
        py::arg("datetime") = py::none(),
        R"(get_profit_percent_yearly(self, datetime=None) -> list[tuple[Datetime, double]]

    Get the account profit percentage (yearly) of the account at the specified deadline,
    excluding the external cash flows (check in / check out)

    :param Datetime datetime: the specified deadline (defaults to the current time)
    :return: the account profit percentage (yearly))")

        DEF_PICKLE(TradeManagerPtr);
}
