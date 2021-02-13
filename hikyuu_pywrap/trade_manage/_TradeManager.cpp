/*
 * _TradeManager.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_manage/build_in.h>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

#if HKU_PYTHON_SUPPORT_PICKLE
struct TradeManager_pickle_suite : bp::pickle_suite {
    static boost::python::tuple getinitargs(TradeManager const& w) {
        return boost::python::make_tuple(w.initDatetime(), w.initCash(), w.costFunc(), w.name());
    }

    static bp::object getstate(const TradeManager& param) {
        std::ostringstream os;
        OUTPUT_ARCHIVE oa(os);
        oa << param;
        return bp::str(os.str());
    }

    static void setstate(TradeManager& params, bp::object entries) {
        bp::str s = bp::extract<bp::str>(entries)();
        std::string st = bp::extract<std::string>(s)();
        std::istringstream is(st);

        INPUT_ARCHIVE ia(is);
        ia >> params;
    }
};
#endif /* HKU_PYTHON_SUPPORT_PICKLE */

FundsRecord (TradeManager::*getFunds_1)(KQuery::KType) const = &TradeManager::getFunds;
FundsRecord (TradeManager::*getFunds_2)(const Datetime&, KQuery::KType) = &TradeManager::getFunds;

PriceList (TradeManager::*getTMFundsCurve_1)(const DatetimeList&,
                                             KQuery::KType) = &TradeManager::getFundsCurve;
PriceList (TradeManager::*getTMFundsCurve_2)() = &TradeManager::getFundsCurve;

PriceList (TradeManager::*getTMProfitCurve_1)(const DatetimeList&,
                                              KQuery::KType ktype) = &TradeManager::getProfitCurve;
PriceList (TradeManager::*getTMProfitCurve_2)() = &TradeManager::getProfitCurve;

TradeCostPtr (TradeManager::*get_costFunc)() const = &TradeManager::costFunc;
void (TradeManager::*set_costFunc)(const TradeCostPtr&) = &TradeManager::costFunc;

string (TradeManager::*tm_get_name)() const = &TradeManager::name;
void (TradeManager::*tm_set_name)(const string&) = &TradeManager::name;

const TradeRecordList& (TradeManager::*_getTradeList_1)() const = &TradeManager::getTradeList;
TradeRecordList (TradeManager::*_getTradeList_2)(const Datetime&, const Datetime&) const =
  &TradeManager::getTradeList;

void export_TradeManager() {
    // size_t null_size = Null<size_t>();

    class_<TradeManager>(
      "TradeManager",
      R"(交易管理类，可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。

交易管理可理解为一个模拟账户进行模拟交易。一般使用 crtTM 创建交易管理实例。

公共参数：

    - reinvest=False (bool) : 红利是否再投资
    - precision=2 (int) : 价格计算精度
    - support_borrow_cash=False (bool) : 是否自动融资
    - support_borrow_stock=False (bool) : 是否自动融券
    - save_action=True (bool) : 是否保存Python命令序列)",
      init<const Datetime&, price_t, const TradeCostPtr&, const string&>())

      .def("__str__", &TradeManager::toString)
      .def("__repr__", &TradeManager::toString)

      .add_property("name", tm_get_name, tm_set_name, "名称")
      .add_property("init_cash", &TradeManager::initCash, "（只读）初始资金")
      .add_property("current_cash", &TradeManager::currentCash, "（只读）当前资金")
      .add_property("init_datetime", &TradeManager::initDatetime, "（只读）账户建立日期")
      .add_property("first_datetime", &TradeManager::firstDatetime,
                    "（只读）第一笔买入交易发生日期，如未发生交易返回 Datetime>()")
      .add_property("last_datetime", &TradeManager::lastDatetime,
                    "（只读）最后一笔交易日期，注意和交易类型无关，如未发生交易返回账户建立日期")
      .add_property("precision", &TradeManager::precision,
                    "（只读）价格精度，同公共参数“precision”")
      .add_property("cost_func", get_costFunc, set_costFunc, "交易成本算法")
      .add_property("broker_last_datetime", &TradeManager::getBrokerLastDatetime,
                    &TradeManager::setBrokerLastDatetime,
                    R"(实际开始订单代理操作的时刻。
        
    默认情况下，TradeManager会在执行买入/卖出操作时，调用订单代理执行代理的买入/卖出动作，但这样在实盘操作时会存在问题。因为系统在计算信号指示时，需要回溯历史数据才能得到最新的信号，这样TradeManager会在历史时刻就执行买入/卖出操作，此时如果订单代理本身没有对发出买入/卖出指令的时刻进行控制，会导致代理发送错误的指令。此时，需要指定在某一个时刻之后，才允许指定订单代理的买入/卖出操作。属性 brokeLastDatetime 即用于指定该时刻。)")

      .def("getParam", &TradeManager::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &TradeManager::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string | Query | KData | Stock | DatetimeList
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &TradeManager::haveParam, "是否存在指定参数")

      .def("reset", &TradeManager::reset, "复位，清空交易、持仓记录")
      .def("clone", &TradeManager::clone, "克隆（深复制）实例")

      .def("reg_broker", &TradeManager::regBroker, R"(reg_broker(self, broker)
    
    注册订单代理。可执行多次该命令注册多个订单代理。
        
    :param OrderBrokerBase broker: 订单代理实例)")

      .def("clear_broker", &TradeManager::clearBroker, R"(clear_broker(self)

    清空所有已注册订单代理)")

      //.def("getMarginRate", &TradeManager::getMarginRate)

      .def("have", &TradeManager::have, R"(have(self, stock)

    当前是否持有指定的证券

    :param Stock stock: 指定证券
    :rtype: bool)")

      .def("get_stock_num", &TradeManager::getStockNumber, R"(get_stock_num(self)

    当前持有的证券种类数量，即当前持有几只股票（非各个股票的持仓数）

    :rtype: int)")

      //.def("getShortStockNumber", &TradeManager::getShortStockNumber)

      .def("get_hold_num", &TradeManager::getHoldNumber, R"(get_hold_num(self, datetime, stock)

        获取指定时刻指定证券的持有数量
        
        :param Datetime datetime: 指定时刻
        :param Stock stock: 指定的证券
        :rtype: int)")

      //.def("getShortHoldNumber", &TradeManager::getShortHoldNumber)

      .def("get_trade_list", _getTradeList_1, return_value_policy<copy_const_reference>())
      .def("get_trade_list", _getTradeList_2, R"(get_trade_list(self[, start, end])

    获取交易记录，未指定参数时，获取全部交易记录

    :param Datetime start: 起始日期
    :param Datetime end: 结束日期
    :rtype: TradeRecordList)")

      .def("get_position_list", &TradeManager::getPositionList, R"(get_position_list(self)

    获取当前全部持仓记录

    :rtype: PositionRecordList)")

      .def("get_history_position_list", &TradeManager::getHistoryPositionList,
           return_value_policy<copy_const_reference>(), R"(get_history_position_list(self)

    获取全部历史持仓记录，即已平仓记录

    :rtype: PositionRecordList)")

      .def("get_position", &TradeManager::getPosition, R"(get_position(self, stock)

    获取指定证券的当前持仓记录，如当前未持有该票，返回PositionRecord()

    :param Stock stock: 指定的证券
    :rtype: PositionRecord)")

      .def("get_buy_cost", &TradeManager::getBuyCost,
           R"(get_buy_cost(self, datetime, stock, price, num)

    计算买入成本

    :param Datetime datetime: 交易时间
    :param Stock stock:       交易的证券
    :param float price:       买入价格
    :param float num:         买入数量
    :rtype: CostRecord)")

      .def("get_sell_cost", &TradeManager::getSellCost,
           R"(get_sell_cost(self, datetime, stock, price, num)

    计算卖出成本

    :param Datetime datetime: 交易时间
    :param Stock stock:       交易的证券
    :param float price:       卖出价格
    :param float num:         卖出数量
    :rtype: CostRecord)")

      //.def("getBorrowCashCost", &TradeManager::getBorrowCashCost)
      //.def("getReturnCashCost", &TradeManager::getReturnCashCost)
      //.def("getBorrowStockCost", &TradeManager::getBorrowStockCost)
      //.def("getReturnStockCost", &TradeManager::getReturnStockCost)

      .def("cash", &TradeManager::cash, (arg("datetime"), arg("ktype") = KQuery::DAY),
           R"(cash(self, datetime[, ktype=Query.KType.DAY])

    获取指定时刻的现金。（注：如果不带日期参数，无法根据权息信息调整持仓。）

    :param Datetime datetime: 指定时刻
    :param ktype: K线类型
    :rtype: float)")

      .def("get_funds", getFunds_1, (arg("ktype") = KQuery::DAY))
      .def("get_funds", getFunds_2, (arg("datetime"), arg("ktype") = KQuery::DAY),
           R"(get_funds(self, [datetime, ktype = Query.DAY])

    获取指定时刻的资产市值详情

    :param Datetime datetime:  指定时刻
    :param Query.KType ktype: K线类型
    :rtype: FundsRecord)")

      //.def("getFunds", getFunds_1, (arg("ktype") = KQuery::DAY))
      //.def("getFunds", getFunds_2, (arg("datetime"), arg("ktype") = KQuery::DAY))
      .def("get_funds_curve", getTMFundsCurve_1, (arg("dates"), arg("ktype") = KQuery::DAY),
           R"(get_funds_curve(self, dates[, ktype = Query.DAY])

    获取资产净值曲线

    :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的资产净值曲线
    :param Query.KType ktype: K线类型，必须与日期列表匹配
    :return: 资产净值列表
    :rtype: PriceList)")

      .def("get_funds_curve", getTMFundsCurve_2,
           R"(get_funds_curve(self)

    获取从账户建立日期到系统当前日期的资产净值曲线（按自然日）

    :return: 资产净值列表
    :rtype: PriceList)")

      .def("get_profit_curve", getTMProfitCurve_1, (arg("dates"), arg("ktype") = KQuery::DAY),
           R"(get_profit_curve(self, dates[, ktype = Query.DAY])

    获取收益曲线，即扣除历次存入资金后的资产净值曲线

    :param DatetimeList dates: 日期列表，根据该日期列表获取其对应的收益曲线，应为递增顺序
    :param Query.KType ktype: K线类型，必须与日期列表匹配
    :return: 收益曲线
    :rtype: PriceList)")

      .def("get_profit_curve", getTMProfitCurve_2,
           R"(get_profit_curve(self)

    获取获取从账户建立日期到系统当前日期的收益曲线，即扣除历次存入资金后的资产净值曲线

    :return: 收益曲线
    :rtype: PriceList)")

      .def("checkin", &TradeManager::checkin, R"(checkin(self, datetime, cash)

    向账户内存入现金

    :param Datetime datetime: 交易时间
    :param float cash: 存入的现金量
    :rtype: TradeRecord)")

      .def("checkout", &TradeManager::checkout, R"(checkout(self, datetime, cash)

    从账户内取出现金

    :param Datetime datetime: 交易时间
    :param float cash: 取出的资金量
    :rtype: TradeRecord)")

      //.def("checkinStock", &TradeManager::checkinStock)
      //.def("checkoutStock", &TradeManager::checkoutStock)
      //.def("borrowCash", &TradeManager::borrowCash)
      //.def("returnCash", &TradeManager::returnCash)
      //.def("borrowStock", &TradeManager::borrowStock)
      //.def("returnStock", &TradeManager::returnStock)

      .def(
        "buy", &TradeManager::buy,
        (arg("datetime"), arg("stock"), arg("real_price"), arg("num"), arg("stoploss") = 0.0,
         arg("goal_price") = 0.0, arg("plan_price") = 0.0, arg("part") = PART_INVALID),
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

      // buy_overload(args("datetime", "stock", "realPrice",
      //"num", "stoploss", "goalPrice", "planPrice","part")))

      .def(
        "sell", &TradeManager::sell,
        (arg("datetime"), arg("stock"), arg("real_price"), arg("num") = MAX_DOUBLE,
         arg("stoploss") = 0.0, arg("goal_price") = 0.0, arg("plan_price") = 0.0,
         arg("part") = PART_INVALID),
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
      // sell_overload(args("datetime", "stock", "realPrice", "num", "stoploss", "goalPrice",
      //                   "planPrice", "part")))
      //.def("buyShort", &TradeManager::buyShort, buyShort_overload())
      //.def("sellShort", &TradeManager::sellShort, sellShort_overload())

      .def("add_trade_record", &TradeManager::addTradeRecord, R"(add_trade_record(self, tr)

    直接加入交易记录，如果加入初始化账户记录，将清除全部已有交易及持仓记录。

    :param TradeRecord tr: 交易记录
    :return: True（成功） | False（失败）
    :rtype: bool)")

      .def("tocsv", &TradeManager::tocsv, R"(tocsv(self, path)

    以csv格式输出交易记录、未平仓记录、已平仓记录、资产净值曲线

    :param str path: 输出文件所在目录)")

#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(TradeManager_pickle_suite())
#endif
      ;

    register_ptr_to_python<TradeManagerPtr>();
}
