/*
 * _TradeManager.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <hikyuu/trade_manage/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

class PyTradeManagerBase : public TradeManagerBase {
    PY_CLONE(PyTradeManagerBase, TradeManagerBase)

public:
    using TradeManagerBase::TradeManagerBase;

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
                    price_t stoploss, price_t goalPrice, price_t planPrice,
                    SystemPart from) override {
        PYBIND11_OVERLOAD(TradeRecord, TradeManagerBase, buy, datetime, stock, realPrice, number,
                          stoploss, goalPrice, planPrice, from);
    }

    TradeRecord sell(const Datetime& datetime, const Stock& stock, price_t realPrice, double number,
                     price_t stoploss, price_t goalPrice, price_t planPrice,
                     SystemPart from) override {
        PYBIND11_OVERLOAD(TradeRecord, TradeManagerBase, sell, datetime, stock, realPrice, number,
                          stoploss, goalPrice, planPrice, from);
    }

    TradeRecord sellShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                          double number, price_t stoploss, price_t goalPrice, price_t planPrice,
                          SystemPart from) override {
        PYBIND11_OVERRIDE_NAME(TradeRecord, TradeManagerBase, "sell_short", sellShort, datetime,
                               stock, realPrice, number, stoploss, goalPrice, planPrice, from);
    }

    TradeRecord buyShort(const Datetime& datetime, const Stock& stock, price_t realPrice,
                         double number, price_t stoploss, price_t goalPrice, price_t planPrice,
                         SystemPart from) override {
        PYBIND11_OVERRIDE_NAME(TradeRecord, TradeManagerBase, "buy_short", buyShort, datetime,
                               stock, realPrice, number, stoploss, goalPrice, planPrice, from);
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

    void fetchAssetInfoFromBroker(const OrderBrokerPtr& broker) override {
        PYBIND11_OVERRIDE_NAME(void, TradeManagerBase, "fetch_asset_info_from_broker",
                               fetchAssetInfoFromBroker, broker);
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
      m, "TradeManager",
      R"(交易管理类，可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。

交易管理可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。

公共参数：

    - precision=2 (int) : 价格计算精度
    - support_borrow_cash=False (bool) : 是否自动融资
    - support_borrow_stock=False (bool) : 是否自动融券
    - save_action=True (bool) : 是否保存Python命令序列)")
      .def(py::init<>())
      .def(py::init<const string&, const TradeCostPtr&>())

      .def("__str__", &TradeManagerBase::str)
      .def("__repr__", &TradeManagerBase::str)

      .def_property("name", py::overload_cast<>(&TradeManagerBase::name, py::const_),
                    py::overload_cast<const string&>(&TradeManagerBase::name),
                    py::return_value_policy::copy, "名称")

      .def_property_readonly("init_cash", &TradeManagerBase::initCash, "（只读）初始资金")
      .def_property_readonly("current_cash", &TradeManagerBase::currentCash, "（只读）当前资金")
      .def_property_readonly("init_datetime", &TradeManagerBase::initDatetime,
                             "（只读）账户建立日期")

      .def_property_readonly("first_datetime", &TradeManagerBase::firstDatetime,
                             "（只读）第一笔买入交易发生日期，如未发生交易返回 Datetime>()")
      .def_property_readonly(
        "last_datetime", &TradeManagerBase::lastDatetime,
        "（只读）最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期")
      .def_property_readonly("precision", &TradeManagerBase::precision,
                             "（只读）价格精度，同公共参数“precision”")

      .def_property("cost_func", get_costFunc, set_costFunc, "交易成本算法")

      .def_property("broker_last_datetime", &TradeManagerBase::getBrokerLastDatetime,
                    &TradeManagerBase::setBrokerLastDatetime,
                    R"(实际开始订单代理操作的时刻。
        
    默认情况下，TradeManager会在执行买入/卖出操作时，调用订单代理执行代理的买入/卖出动作，但这样在实盘操作时会存在问题。因为系统在计算信号指示时，需要回溯历史数据才能得到最新的信号，这样TradeManager会在历史时刻就执行买入/卖出操作，此时如果订单代理本身没有对发出买入/卖出指令的时刻进行控制，会导致代理发送错误的指令。此时，需要指定在某一个时刻之后，才允许指定订单代理的买入/卖出操作。属性 brokeLastDatetime 即用于指定该时刻。)")

      .def("getParam", &TradeManagerBase::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &TradeManagerBase::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string | Query | KData | Stock | DatetimeList
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &TradeManagerBase::haveParam, "是否存在指定参数")

      .def("reset", &TradeManagerBase::reset, "复位，清空交易、持仓记录")
      .def("clone", &TradeManagerBase::clone, "克隆（深复制）实例")

      .def("reg_broker", &TradeManagerBase::regBroker, R"(reg_broker(self, broker)
    
    注册订单代理。可执行多次该命令注册多个订单代理。
        
    :param OrderBrokerBase broker: 订单代理实例)")

      .def("clear_broker", &TradeManagerBase::clearBroker, R"(clear_broker(self)

    清空所有已注册订单代理)")

      .def("get_margin_rate", &TradeManagerBase::getMarginRate)

      .def("have", &TradeManagerBase::have, R"(have(self, stock)

    当前是否持有指定的证券

    :param Stock stock: 指定证券
    :rtype: bool)")

      .def("get_stock_num", &TradeManagerBase::getStockNumber, R"(get_stock_num(self)

    当前持有的证券种类数量，即当前持有几只股票（非各个股票的持仓数）

    :rtype: int)")

      .def("get_short_stock_num", &TradeManagerBase::getShortStockNumber)

      .def("get_hold_num", &TradeManagerBase::getHoldNumber, R"(get_hold_num(self, datetime, stock)

        获取指定时刻指定证券的持有数量
        
        :param Datetime datetime: 指定时刻
        :param Stock stock: 指定的证券
        :rtype: int)")

      .def("get_short_hold_num", &TradeManagerBase::getShortHoldNumber)

      .def("get_trade_list", _getTradeList_1)
      .def("get_trade_list", _getTradeList_2, R"(get_trade_list(self[, start, end])

    获取交易记录，未指定参数时，获取全部交易记录

    :param Datetime start: 起始日期
    :param Datetime end: 结束日期
    :rtype: TradeRecordList)")

      .def("get_position_list", &TradeManagerBase::getPositionList, R"(get_position_list(self)

    获取当前全部持仓记录

    :rtype: PositionRecordList)")

      .def("get_history_position_list", &TradeManagerBase::getHistoryPositionList,
           R"(get_history_position_list(self)

    获取全部历史持仓记录，即已平仓记录

    :rtype: PositionRecordList)")

      .def("get_position", &TradeManagerBase::getPosition, R"(get_position(self, date, stock)

    获取指定日期指定证券的持仓记录，如当前未持有该票，返回PositionRecord()

    :param Datetime date: 指定日期
    :param Stock stock: 指定的证券
    :rtype: PositionRecord)")

      .def("get_buy_cost", &TradeManagerBase::getBuyCost,
           R"(get_buy_cost(self, datetime, stock, price, num)

    计算买入成本

    :param Datetime datetime: 交易时间
    :param Stock stock:       交易的证券
    :param float price:       买入价格
    :param float num:         买入数量
    :rtype: CostRecord)")

      .def("get_sell_cost", &TradeManagerBase::getSellCost,
           R"(get_sell_cost(self, datetime, stock, price, num)

    计算卖出成本

    :param Datetime datetime: 交易时间
    :param Stock stock:       交易的证券
    :param float price:       卖出价格
    :param float num:         卖出数量
    :rtype: CostRecord)")

      .def("get_borrow_cash_cost", &TradeManagerBase::getBorrowCashCost)
      .def("get_return_cash_cost", &TradeManagerBase::getReturnCashCost)
      .def("get_borrow_stock_cost", &TradeManagerBase::getBorrowStockCost)
      .def("get_return_stock_cost", &TradeManagerBase::getReturnStockCost)

      .def("cash", &TradeManagerBase::cash, py::arg("datetime"), py::arg("ktype") = KQuery::DAY,
           R"(cash(self, datetime[, ktype=Query.KType.DAY])

    获取指定时刻的现金。（注：如果不带日期参数，无法根据权息信息调整持仓。）

    :param Datetime datetime: 指定时刻
    :param ktype: K线类型
    :rtype: float)")

      .def("get_funds", getFunds_1, py::arg("ktype") = KQuery::DAY)
      .def("get_funds", getFunds_2, py::arg("datetime"), py::arg("ktype") = KQuery::DAY,
           R"(get_funds(self, [datetime, ktype = Query.DAY])

    获取指定时刻的资产市值详情

    :param Datetime datetime:  指定时刻
    :param Query.KType ktype: K线类型
    :rtype: FundsRecord)")

      .def("get_funds_list", &TradeManagerBase::getFundsList, py::arg("dates"),
           py::arg("ktype") = KQuery::DAY, R"(get_funds_list(self, dates[, ktype = Query.DAY])
    
    获取指定日期列表的每日资产记录
    :param Datetime datetime:  指定时刻
    :param Query.KType ktype: K线类型
    :rtype: FundsList)")

      .def("get_funds_curve", &TradeManagerBase::getFundsCurve, py::arg("dates"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_funds_curve(self, dates[, ktype = Query.DAY])

    获取资产净值曲线

    :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的资产净值曲线
    :param Query.KType ktype: K线类型，必须与日期列表匹配
    :return: 资产净值列表
    :rtype: PriceList)")

      .def("get_profit_curve", &TradeManagerBase::getProfitCurve, py::arg("dates"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_profit_curve(self, dates[, ktype = Query.DAY])

    获取收益曲线，即扣除历次存入资金后的资产净值曲线

    :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
    :param Query.KType ktype: K线类型，必须与日期列表匹配
    :return: 收益曲线
    :rtype: PriceList)")

      .def("get_profit_cum_change_curve", &TradeManagerBase::getProfitCumChangeCurve,
           py::arg("dates"), py::arg("ktype") = KQuery::DAY,
           R"(get_profit_cum_change_curve(self, dates[, ktype = Query.DAY])

    获取累积收益率曲线

    :param DatetimeList dates: 日期列表
    :param Query.KType ktype: K线类型，必须与日期列表匹配
    :rtype: PriceList)")

      .def("get_base_assets_curve", &TradeManagerBase::getBaseAssetsCurve, py::arg("dates"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_profit_curve(self, dates[, ktype = Query.DAY])

    获取投入本值资产曲线（投入本钱）

    :param DatetimeList dates: 日期列表
    :param Query.KType ktype: K线类型，必须与日期列表匹配
    :rtype: PriceList)")

      .def("checkin", &TradeManagerBase::checkin, R"(checkin(self, datetime, cash)

    向账户内存入现金

    :param Datetime datetime: 交易时间
    :param float cash: 存入的现金量
    :rtype: TradeRecord)")

      .def("checkout", &TradeManagerBase::checkout, R"(checkout(self, datetime, cash)

    从账户内取出现金

    :param Datetime datetime: 交易时间
    :param float cash: 取出的资金量
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
        py::arg("plan_price") = 0.0, py::arg("part") = PART_INVALID,
        R"(buy(self, datetime, stock, real_price, number[, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID])

    买入操作

    :param Datetime datetime: 买入时间
    :param Stock stock:       买入的证券
    :param float real_price:  实际买入价格
    :param float num:         买入数量
    :param float stoploss:    止损价
    :param float goal_price:  目标价格
    :param float plan_price:  计划买入价格
    :param SystemPart part:   交易指示来源
    :rtype: TradeRecord)")

      .def(
        "sell", &TradeManagerBase::sell, py::arg("datetime"), py::arg("stock"),
        py::arg("real_price"), py::arg("num") = MAX_DOUBLE, py::arg("stoploss") = 0.0,
        py::arg("goal_price") = 0.0, py::arg("plan_price") = 0.0, py::arg("part") = PART_INVALID,
        R"(sell(self, datetime, stock, realPrice[, number=constant.max_double, stoploss=0.0, goal_price=0.0, plan_price=0.0, part=System.INVALID])

    卖出操作

    :param Datetime datetime: 卖出时间
    :param Stock stock:       卖出的证券
    :param float real_price:  实际卖出价格
    :param float num:         卖出数量，如果等于constant.max_double，表示全部卖出
    :param float stoploss:    新的止损价
    :param float goal_price:  新的目标价格
    :param float plan_price:  原计划卖出价格
    :param SystemPart part:   交易指示来源
    :rtype: TradeRecord)")

      .def("buy_short", &TradeManagerBase::buyShort)
      .def("sell_short", &TradeManagerBase::sellShort)

      .def("add_trade_record", &TradeManagerBase::addTradeRecord, R"(add_trade_record(self, tr)

    直接加入交易记录，如果加入初始化账户记录，将清除全部已有交易及持仓记录。

    :param TradeRecord tr: 交易记录
    :return: True（成功） | False（失败）
    :rtype: bool)")

      .def("add_position", &TradeManagerBase::addPosition, R"(add_postion(self, position)

    建立初始账户后，直接加入持仓记录，仅用于构建初始有持仓的账户

    :param PositionRecord position: 持仓记录
    return True | False)")

      .def("tocsv", &TradeManagerBase::tocsv, R"(tocsv(self, path)

    以csv格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线

    :param str path: 输出文件所在目录)")

      .def("update_with_weight", &TradeManagerBase::updateWithWeight,
           R"(update_with_weight(self, date)

      根据权息信息更新当前持仓及交易记录，必须按时间顺序被调用

      :param Datetime date: 当前时刻)")

      .def("fetch_asset_info_from_broker", &TradeManagerBase::fetchAssetInfoFromBroker)

        DEF_PICKLE(TradeManagerPtr);
}
