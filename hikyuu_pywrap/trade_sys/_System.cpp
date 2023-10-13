/*
 * _System.cpp
 *
 *  Created on: 2013-3-22
 *      Author: fasiondog
 */

#include <boost/python.hpp>
#include <hikyuu/trade_sys/system/build_in.h>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include "../_Parameter.h"
#include "../pickle_support.h"

using namespace boost::python;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(run_overload, run, 2, 3);

const string& (System::*sys_get_name)() const = &System::name;
void (System::*sys_set_name)(const string&) = &System::name;

void (System::*run_1)(const KQuery&, bool) = &System::run;
void (System::*run_2)(const KData&, bool) = &System::run;
void (System::*run_3)(const Stock&, const KQuery&, bool reset) = &System::run;

TradeRecord (System::*runMoment_1)(const Datetime&) = &System::runMoment;

void export_System() {
    def(
      "SYS_Simple", SYS_Simple,
      (arg("tm") = TradeManagerPtr(), arg("mm") = MoneyManagerPtr(), arg("ev") = EnvironmentPtr(),
       arg("cn") = ConditionPtr(), arg("sg") = SignalPtr(), arg("st") = StoplossPtr(),
       arg("tp") = StoplossPtr(), arg("pg") = ProfitGoalPtr(), arg("sp") = SlippagePtr()),
      R"(SYS_Simple([tm=None, mm=None, ev=None, cn=None, sg=None, st=None, tp=None, pg=None, sp=None])

  创建简单系统实例（每次交易不进行多次加仓或减仓，即每次买入后在卖出时全部卖出），  系统实例在运行时(调用run方法），至少需要一个配套的交易管理实例、一个资金管理策略
  和一个信号指示器），可以在创建系统实例后进行指定。如果出现调用run时没有任何输出，
  且没有正确结果的时候，可能是未设置tm、sg、mm。进行回测时，使用 run 方法，如::
    
        #创建模拟交易账户进行回测，初始资金30万
        my_tm = crtTM(init_cash = 300000)

        #创建信号指示器（以5日EMA为快线，5日EMA自身的10日EMA作为慢线，快线向上穿越慢线时买入，反之卖出）
        my_sg = SG_Flex(EMA(C, n=5), slow_n=10)

        #固定每次买入1000股
        my_mm = MM_FixedCount(1000)

        #创建交易系统并运行
        sys = SYS_Simple(tm = my_tm, sg = my_sg, mm = my_mm)
        sys.run(sm['sz000001'], Query(-150))
    
    :param TradeManager tm: 交易管理实例 
    :param MoneyManager mm: 资金管理策略
    :param EnvironmentBase ev: 市场环境判断策略
    :param ConditionBase cn: 系统有效条件
    :param SignalBase sg: 信号指示器
    :param StoplossBase st: 止损策略
    :param StoplossBase tp: 止盈策略
    :param ProfitGoalBase pg: 盈利目标策略
    :param SlippageBase sp: 移滑价差算法
    :return: system实例)");

    def("get_system_part_name", getSystemPartName, R"(get_system_part_name(part)

    获取部件的字符串名称
    
        - System.Part.ENVIRONMENT  - "EV"
        - System.Part.CONDITION    - "CN"
        - System.Part.SIGNAL       - "SG"
        - System.Part.STOPLOSS     - "ST"
        - System.Part.TAKEPROFIT   - "TP"
        - System.Part.MONEYMANAGER - "MM"
        - System.Part.PROFITGOAL   - "PG"
        - System.Part.SLIPPAGE     - "SP"
        - System.Part.INVALID      - "--"

    :param int part: System.Part 枚举值
    :rtype: str)");

    def("get_system_part_enum", getSystemPartEnum, R"(get_system_part_enum(part_name)

     根据系统部件的字符串名称获取相应的枚举值

    :param str part_name: 系统部件的字符串名称，参见：:py:func:`getSystemPartName`
    :rtype: System.Part)");

    class_<TradeRequest>(
      "TradeRequest",
      R"(交易请求记录。系统内部在实现延迟操作时登记的交易请求信息。暴露该结构的主要目的是用于在“delay”模式（延迟到下一个bar开盘时进行交易）的情况下，系统实际已知下一个Bar将要进行交易，此时可通过 :py:meth:`System.getBuyTradeRequest` 、 :py:meth:`System.getSellTradeRequest` 来获知下一个BAR是否需要买入/卖出。主要用于提醒或打印下一个Bar需要进行操作。对于系统本身的运行没有影响。)",
      init<>())

      //.def(self_ns::str(self))
      //.def(self_ns::repr(self))

      .def_readwrite("valid", &TradeRequest::valid, "该交易请求记录是否有效（True | False）")
      .def_readwrite("business", &TradeRequest::business,
                     "交易业务类型，参见：:py:class:`hikyuu.trade_manage.BUSINESS`")
      .def_readwrite("datetime", &TradeRequest::datetime, "发出交易请求的时刻")
      .def_readwrite("stoploss", &TradeRequest::stoploss, "发出交易请求时刻的止损价")
      .def_readwrite("part", &TradeRequest::from,
                     "发出交易请求的来源，参见：:py:class:`System.Part`")
      .def_readwrite("count", &TradeRequest::count, "因操作失败，连续延迟的次数")
#if HKU_PYTHON_SUPPORT_PICKLE
      .def_pickle(normal_pickle_suite<TradeRequest>())
#endif
      ;

    class_<SystemList>("SystemList").def(vector_indexing_suite<SystemList>());

    scope in_System =
      class_<System>(
        "System",
        R"(系统基类。需要扩展或实现更复杂的系统交易行为，可从此类继承。

系统是指针对单个交易对象的完整策略，包括环境判断、系统有效条件、资金管理、止损、止盈、盈利目标、移滑价差的完整策略，用于模拟回测。

公共参数：

  - delay=True (bool) : 是否延迟到下一个bar开盘时进行交易
  - delay_use_current_price=True (bool) : 延迟操作的情况下，是使用当前交易时bar的价格计算新的止损价/止赢价/目标价还是使用上次计算的结果
  - max_delay_count=3 (int) : 连续延迟交易请求的限制次数，应大于等于0，0表示只允许延迟1次
  - tp_monotonic=True (bool) : 止赢单调递增
  - tp_delay_n=3 (int) : 止盈延迟开始的天数，即止盈策略判断从实际交易几天后开始生效
  - ignore_sell_sg=False (bool) : 忽略卖出信号，只使用止损/止赢等其他方式卖出
  - ev_open_position=False (bool): 是否使用市场环境判定进行初始建仓
  - cn_open_position=False (bool): 是否使用系统有效性条件进行初始建仓)",
        init<const string&>())
        .def(init<const TradeManagerPtr&, const MoneyManagerPtr&, const EnvironmentPtr&,
                  const ConditionPtr&, const SignalPtr&, const StoplossPtr&, const StoplossPtr&,
                  const ProfitGoalPtr&, const SlippagePtr&, const string&>())
        .def(self_ns::str(self))
        .def(self_ns::repr(self))

        .add_property("name",
                      make_function(sys_get_name, return_value_policy<copy_const_reference>()),
                      sys_set_name, "系统名称")
        .add_property("tm", &System::getTM, &System::setTM, "关联的交易管理实例")
        .add_property("to", &System::getTO, &System::setTO, "交易对象 KData")
        .add_property("mm", &System::getMM, &System::setMM, "资金管理策略")
        .add_property("ev", &System::getEV, &System::setEV, "市场环境判断策略")
        .add_property("cn", &System::getCN, &System::setCN, "系统有效条件")
        .add_property("sg", &System::getSG, &System::setSG, "信号指示器")
        .add_property("st", &System::getST, &System::setST, "止损策略")
        .add_property("tp", &System::getTP, &System::setTP, "止盈策略")
        .add_property("pg", &System::getPG, &System::setPG, "盈利目标策略")
        .add_property("sp", &System::getSP, &System::setSP, "移滑价差算法")

        .def("get_param", &System::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

        .def("set_param", &System::setParam<object>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

        .def("have_param", &System::haveParam, "是否存在指定参数")

        .def("get_stock", &System::getStock, R"(get_stock(self)

    获取关联的证券

    :rtype: Stock)")

        .def("get_trade_record_list", &System::getTradeRecordList,
             return_value_policy<copy_const_reference>(), R"(get_trade_record_list(self)

    获取实际执行的交易记录，和 TM 的区别是不包含权息调整带来的交易记录

    :rtype: TradeRecordList)")

        .def("get_buy_trade_request", &System::getBuyTradeRequest,
             return_value_policy<copy_const_reference>(), R"(get_buy_trade_request(self)
  
    获取买入请求，“delay”模式下查看下一时刻是否存在买入操作

    :rtype: TradeRequest)")

        .def("get_sell_trade_request", &System::getSellTradeRequest,
             return_value_policy<copy_const_reference>(), R"(get_sell_trade_request(self)

    获取卖出请求，“delay”模式下查看下一时刻是否存在卖出操作

    :rtype: TradeRequest)")

        /*.def("getSellShortTradeRequest", &System::getSellShortTradeRequest,
             return_value_policy<copy_const_reference>())
        .def("getBuyShortTradeRequest", &System::getBuyShortTradeRequest,
             return_value_policy<copy_const_reference>())*/

        .def("reset", &System::reset, R"(reset(self, with_tm, with_ev)

    复位操作。TM、EV是和具体系统无关的策略组件，可以在不同的系统中进行共享，复位将引起系统运行时被重新清空并计算。尤其是在共享TM时需要注意！

    :param bool with_tm: 是否复位TM组件
    :param bool with_ev: 是否复位EV组件)")

        .def("clone", &System::clone, R"(clone(self)

    克隆操作。)")

        //.def("run", &System::run, run_overload(args("stock", "query", "reset")))
        .def("run", run_1, (arg("query"), arg("reset") = true))
        .def("run", run_2, (arg("kdata"), arg("reset") = true))
        .def("run", run_3, (arg("stock"), arg("query"), arg("reset") = true),
             R"(run(self, stock, query[, reset=True])
  
    运行系统，执行回测

    :param Stock stock: 交易的证券
    :param Query query: K线数据查询条件
    :param bool reset: 是否同时复位所有组件，尤其是tm实例)")

        .def("ready", &System::readyForRun)

#if HKU_PYTHON_SUPPORT_PICKLE
        .def_pickle(name_init_pickle_suite<System>())
#endif
      ;

    enum_<SystemPart>("Part")
      .value("ENVIRONMENT", PART_ENVIRONMENT)
      .value("CONDITION", PART_CONDITION)
      .value("SIGNAL", PART_SIGNAL)
      .value("STOPLOSS", PART_STOPLOSS)
      .value("TAKEPROFIT", PART_TAKEPROFIT)
      .value("MONEYMANAGER", PART_MONEYMANAGER)
      .value("PROFITGOAL", PART_PROFITGOAL)
      .value("SLIPPAGE", PART_SLIPPAGE)
      .value("ALLOCATEFUNDS", PART_ALLOCATEFUNDS)
      .value("INVALID", PART_INVALID);

    register_ptr_to_python<SystemPtr>();
}
