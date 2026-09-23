/*
 * _TradeManager.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/build_in.h>
#include <hikyuu/trade_manage/Performance.h>
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
        PYBIND11_OVERRIDE_NAME(double, TradeManagerBase, "get_debt_num", getDebtNumber, datetime,
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

FundsRecord (TradeManagerBase::*getFunds_1)(KQuery::KType) const = &TradeManagerBase::getFunds;
FundsRecord (TradeManagerBase::*getFunds_2)(const Datetime&,
                                            KQuery::KType) = &TradeManagerBase::getFunds;

TradeCostPtr (TradeManagerBase::*get_costFunc)() const = &TradeManagerBase::costFunc;
void (TradeManagerBase::*set_costFunc)(const TradeCostPtr&) = &TradeManagerBase::costFunc;

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
    - save_action=True (bool) : whether to save the Python command sequences)")
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
                             "has occurred, return Datetime>()")
      .def_property_readonly(
        "last_datetime", &TradeManagerBase::lastDatetime,
        "(Read-only) the date of the last trade; note that it is unrelated to the trade type; if "
        "no trade has occurred, return the account establishment date")
      .def_property_readonly(
        "precision", &TradeManagerBase::precision,
        "(Read-only) the price precision, the same as the common parameter 'precision'")

      .def_property("cost_func", get_costFunc, set_costFunc, "The trade cost algorithm")

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

      .def("get_position", &TradeManagerBase::getPosition, R"(get_position(self, date, stock)

    Get the position record of the security on the specified date; if the stock is not currently held, return PositionRecord()

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

      .def("cash", &TradeManagerBase::cash, py::arg("datetime"), py::arg("ktype") = KQuery::DAY,
           R"(cash(self, datetime[, ktype=Query.KType.DAY])

    Get the cash at the specified moment. (Note: without the date parameter, the positions cannot be adjusted according to the dividend information.)

    :param Datetime datetime: the specified moment
    :param ktype: the K-line type
    :rtype: float)")

      .def("get_funds", getFunds_1, py::arg("ktype") = KQuery::DAY)
      .def("get_funds", getFunds_2, py::arg("datetime"), py::arg("ktype") = KQuery::DAY,
           R"(get_funds(self, [datetime, ktype = Query.DAY])

    Get the asset market value details at the specified moment

    :param Datetime datetime:  the specified moment
    :param Query.KType ktype: the K-line type
    :rtype: FundsRecord)")

      .def("get_funds_list", &TradeManagerBase::getFundsList, py::arg("dates"),
           py::arg("ktype") = KQuery::DAY, R"(get_funds_list(self, dates[, ktype = Query.DAY])
    
    Get the daily asset records of the specified date list
    :param DatetimeList dates:  the specified moments
    :param Query.KType ktype: the K-line type
    :rtype: FundsList)")

      .def("get_funds_curve", &TradeManagerBase::getFundsCurve, py::arg("dates"),
           py::arg("ktype") = KQuery::DAY,
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

    Get the cumulative return curve

    :param DatetimeList dates: the date list
    :param Query.KType ktype: the K-line type, which must match the date list
    :rtype: PriceList)")

      .def("get_base_assets_curve", &TradeManagerBase::getBaseAssetsCurve, py::arg("dates"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_profit_curve(self, dates[, ktype = Query.DAY])

    Get the invested principal asset curve (the invested capital)

    :param DatetimeList dates: the date list
    :param Query.KType ktype: the K-line type, which must match the date list
    :rtype: PriceList)")

      .def("checkin", &TradeManagerBase::checkin, R"(checkin(self, datetime, cash)

    Deposit the cash into the account

    :param Datetime datetime: the trading time
    :param float cash: the amount of the cash deposited
    :rtype: TradeRecord)")

      .def("checkout", &TradeManagerBase::checkout, R"(checkout(self, datetime, cash)

    Withdraw the cash from the account

    :param Datetime datetime: the trading time
    :param float cash: the amount of the funds withdrawn
    :rtype: TradeRecord)")

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
    :param float real_price:  the actual buy price
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
    :param float real_price:  the actual sell price
    :param float num:         the sell quantity; if it equals constant.max_double, it means selling all
    :param float stoploss:    the new stop-loss price
    :param float goal_price:  the new target price
    :param float plan_price:  the originally planned sell price
    :param SystemPart part:   the source of the trading instruction
    :param string remark:     the trade remark
    :rtype: TradeRecord)")

      .def("buy_short", &TradeManagerBase::buyShort)
      .def("sell_short", &TradeManagerBase::sellShort)

      .def("add_trade_record", &TradeManagerBase::addTradeRecord, R"(add_trade_record(self, tr)

    Add the trade record directly; if the initialization account record is added, all the existing trade and position records will be cleared.

    :param TradeRecord tr: the trade record
    :return: True (success) | False (failure)
    :rtype: bool)")

      .def("add_position", &TradeManagerBase::addPosition, R"(add_postion(self, position)

    After establishing the initial account, add the position record directly; it is only used to build an account with the initial positions

    :param PositionRecord position: the position record
    return True | False)")

      .def("tocsv", &TradeManagerBase::tocsv, R"(tocsv(self, path)

    Output the trade records, the open position records, the closed position records and the net asset value curve in the csv format

    :param str path: the directory of the output files)")

      .def("update_with_weight", &TradeManagerBase::updateWithWeight,
           R"(update_with_weight(self, date)

      Update the current positions and the trade records according to the dividend information; it must be called in the chronological order

      :param Datetime date: the current moment)")

      .def("fetch_asset_info_from_broker", &TradeManagerBase::fetchAssetInfoFromBroker,
           py::arg("broker"), py::arg("date") = Datetime(),
           R"(fetch_asset_info_from_broker(self, date)

      Synchronize the asset information at the current moment from the Broker; it must be called in the chronological order

      :param broker the order broker instance
      :param date when synchronizing, it is usually the current time (Null); it can also be forced to a specified time point)")

      .def("get_performance", &TradeManagerBase::getPerformance,
           py::arg("datetime") = Datetime::now(), py::arg("ktype") = KQuery::DAY,
           py::arg("ext") = true,
           R"(get_performance(self[, datetime=Datetime.now(), ktype=Query.DAY]) -> Performance)
        
    Get the account performance at the specified moment of the account

    :param Datetime datetime: the specified moment
    :param Query.KType ktype: the K-line type
    :param bool ext: whether to get the extended statistics items (the donating user); otherwise, they are still the basic statistics items
    :return: the account performance)")

      .def("get_max_pull_back", &TradeManagerBase::getMaxPullBack,
           py::arg("date") = Datetime::now(), py::arg("ktype") = KQuery::DAY,
           R"(get_max_pull_back(self, date, ktype=Query.DAY) -> float
    
    Get the maximum drawdown percentage of the account at the specified moment (a negative number)

    :param Datetime date: the specified date (including this moment)
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
        )")

      .def(
        "get_position_ext_info_list", &TradeManagerBase::getPositionExtInfoList,
        py::arg("current_time"), py::arg("ktype") = KQuery::DAY, py::arg("trade_mode") = 0,
        R"(get_position_ext_info_list(self, current_time, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo])
          
    Get the position details (the open position records) of the specified time after the last trading moment of the account
 
    :param Datetime current_time: the current moment (it needs to be greater than or equal to the last trading moment)
    :param Query.KType ktype: the K-line type
    :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open
    :return: the list of the extended position details)")

      .def(
        "get_position_ext_info", &TradeManagerBase::getPositionExtInfoDict, py::arg("current_time"),
        py::arg("ktype") = KQuery::DAY, py::arg("trade_mode") = 0,
        R"(get_position_ext_info_list(self, current_time, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo])
          
    Get the position details of the specified time after the last trading moment of the account, returned as a dictionary, with the stock as the key and the PositionExtInfo as the value
 
    :param Datetime current_time: the current moment (it needs to be greater than or equal to the last trading moment)
    :param Query.KType ktype: the K-line type
    :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open
    :return: the list of the extended position details)")

      .def(
        "get_history_position_ext_info_list", &TradeManagerBase::getHistoryPositionExtInfoList,
        py::arg("ktype") = KQuery::DAY, py::arg("trade_mode") = 0,
        R"(get_history_position_ext_info_list(self, ktype=Query.DAY, trade_mode=0) -> list[PositionExtInfo])
          
    Get the historical position extended details of the account (the closed records)
 
    :param Query.KType ktype: the K-line type
    :param int trade_mode: the trading mode, affecting some statistics items: 0-trading at the close, 1-trading at the next open
    :return: the list of the extended position details)")

      .def(
        "get_profit_percent_monthly", &TradeManagerBase::getProfitPercentMonthly,
        py::arg("datetime") = Datetime::now(),
        R"(get_profit_percent_monthly(self, datetime=Datetime.now()) -> list[tuple[Datetime, double]])

    Get the account profit percentage (monthly) of the account at the specified deadline

    :param Datetime datetime: the specified deadline
    :return: the account profit percentage (monthly))")

      .def(
        "get_profit_percent_yearly", &TradeManagerBase::getProfitPercentYearly,
        py::arg("datetime") = Datetime::now(),
        R"(get_profit_percent_yearly(self, datetime=Datetime.now()) -> list[tuple[Datetime, double]])

    Get the account profit percentage (yearly) of the account at the specified deadline

    :param Datetime datetime: the specified deadline
    :return: the account profit percentage (yearly))")

        DEF_PICKLE(TradeManagerPtr);
}