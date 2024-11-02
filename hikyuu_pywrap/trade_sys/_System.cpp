/*
 * _System.cpp
 *
 *  Created on: 2013-3-22
 *      Author: fasiondog
 */

#include <hikyuu/trade_sys/system/build_in.h>
#include "../pybind_utils.h"

namespace py = pybind11;
using namespace hku;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_System(py::module& m) {
    m.def("get_system_part_name", getSystemPartName, R"(get_system_part_name(part)

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

    m.def("get_system_part_enum", getSystemPartEnum, R"(get_system_part_enum(part_name)

     根据系统部件的字符串名称获取相应的枚举值

    :param str part_name: 系统部件的字符串名称，参见：:py:func:`getSystemPartName`
    :rtype: System.Part)");

    //--------------------------------------------------------------------------------------
    py::class_<TradeRequest>(
      m, "TradeRequest",
      R"(交易请求记录。系统内部在实现延迟操作时登记的交易请求信息。暴露该结构的主要目的是用于
在“delay”模式（延迟到下一个bar开盘时进行交易）的情况下，系统实际已知下一个Bar将要
进行交易，此时可通过 System.getBuyTradeRequest() 、 System.getSellTradeRequest()
来获知下一个BAR是否需要买入/卖出。主要用于提醒或打印下一个Bar需要进行操作。对于系统
本身的运行没有影响。)")

      .def(py::init<>())
      .def("__str__", to_py_str<TradeRequest>)
      .def("__repr__", to_py_str<TradeRequest>)

      .def_readwrite("valid", &TradeRequest::valid, "该交易请求记录是否有效（True | False）")
      .def_readwrite("business", &TradeRequest::business,
                     "交易业务类型，参见：:py:class:`hikyuu.trade_manage.BUSINESS`")
      .def_readwrite("datetime", &TradeRequest::datetime, "发出交易请求的时刻")
      .def_readwrite("stoploss", &TradeRequest::stoploss, "发出交易请求时刻的止损价")
      .def_readwrite("part", &TradeRequest::from,
                     "发出交易请求的来源，参见：:py:class:`System.Part`")
      .def_readwrite("count", &TradeRequest::count, "因操作失败，连续延迟的次数")
        DEF_PICKLE(TradeRequest);

    //--------------------------------------------------------------------------------------
    py::class_<System, SystemPtr>(m, "System",
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
  - cn_open_position=False (bool): 是否使用系统有效性条件进行初始建仓)")

      .def(py::init<const string&>())
      .def(py::init<const System&>())
      .def(py::init<const TradeManagerPtr&, const MoneyManagerPtr&, const EnvironmentPtr&,
                    const ConditionPtr&, const SignalPtr&, const StoplossPtr&, const StoplossPtr&,
                    const ProfitGoalPtr&, const SlippagePtr&, const string&>())
      .def("__str__", to_py_str<System>)
      .def("__repr__", to_py_str<System>)

      .def_property("name", py::overload_cast<>(&System::name, py::const_),
                    py::overload_cast<const string&>(&System::name), py::return_value_policy::copy,
                    "系统名称")
      .def_property_readonly("query", &System::getQuery, py::return_value_policy::copy, "查询条件")
      .def_property("tm", &System::getTM, &System::setTM, "关联的交易管理实例")
      .def_property("to", &System::getTO, &System::setTO, "交易对象 KData")
      .def_property("mm", &System::getMM, &System::setMM, "资金管理策略")
      .def_property("ev", &System::getEV, &System::setEV, "市场环境判断策略")
      .def_property("cn", &System::getCN, &System::setCN, "系统有效条件")
      .def_property("sg", &System::getSG, &System::setSG, "信号指示器")
      .def_property("st", &System::getST, &System::setST, "止损策略")
      .def_property("tp", &System::getTP, &System::setTP, "止盈策略")
      .def_property("pg", &System::getPG, &System::setPG, "盈利目标策略")
      .def_property("sp", &System::getSP, &System::setSP, "移滑价差算法")

      .def("get_param", &System::getParam<boost::any>, R"(get_param(self, name)

    获取指定的参数

    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数)")

      .def("set_param", &System::setParam<boost::any>, R"(set_param(self, name, value)

    设置参数

    :param str name: 参数名称
    :param value: 参数值
    :raises logic_error: Unsupported type! 不支持的参数类型)")

      .def("have_param", &System::haveParam, "是否存在指定参数")

      .def("set_not_shared_all", &System::setNotSharedAll, "将所有组件设置为非共享")

      .def("get_stock", &System::getStock, R"(get_stock(self)

    获取关联的证券

    :rtype: Stock)")

      .def("get_trade_record_list", &System::getTradeRecordList, py::return_value_policy::copy,
           R"(get_trade_record_list(self)

    获取实际执行的交易记录，和 TM 的区别是不包含权息调整带来的交易记录

    :rtype: TradeRecordList)")

      .def("get_buy_trade_request", &System::getBuyTradeRequest, py::return_value_policy::copy,
           R"(get_buy_trade_request(self)
  
    获取买入请求，“delay”模式下查看下一时刻是否存在买入操作

    :rtype: TradeRequest)")

      .def("get_sell_trade_request", &System::getSellTradeRequest, py::return_value_policy::copy,
           R"(get_sell_trade_request(self)

    获取卖出请求，“delay”模式下查看下一时刻是否存在卖出操作

    :rtype: TradeRequest)")

      .def("get_sell_short_trade_request", &System::getSellShortTradeRequest,
           py::return_value_policy::copy)
      .def("get_buy_short_trade_request", &System::getBuyShortTradeRequest,
           py::return_value_policy::copy)

      .def("reset", &System::reset,
           R"(reset(self)

    复位，但不包括已有的交易对象，以及共享的部件。)")

      .def("force_reset_all", &System::forceResetAll,
           R"(force_reset_all(self)

    强制复位所有组件以及清空已有的交易对象，忽略组件的共享属性。)")

      .def("clone", &System::clone,
           R"(clone(self)

    克隆操作，会依据部件的共享特性进行克隆，共享部件不进行实际的克隆操作，保持共享。)")

      .def("run", py::overload_cast<const KQuery&, bool, bool>(&System::run), py::arg("query"),
           py::arg("reset") = true, py::arg("reset_all") = false)
      .def("run", py::overload_cast<const KData&, bool, bool>(&System::run), py::arg("kdata"),
           py::arg("reset") = true, py::arg("reset_all") = false)
      .def("run", py::overload_cast<const Stock&, const KQuery&, bool, bool>(&System::run),
           py::arg("stock"), py::arg("query"), py::arg("reset") = true,
           py::arg("reset_all") = false,
           R"(run(self, stock, query[, reset=True])
  
    运行系统，执行回测

    :param Stock stock: 交易的证券
    :param Query query: K线数据查询条件
    :param bool reset: 执行前是否依据系统部件共享属性复位
    :param bool reset_all: 强制复位所有部件)")

      .def("ready", &System::readyForRun)

        DEF_PICKLE(System);

    //--------------------------------------------------------------------------------------
    m.def(
      "SYS_Simple",
      [](py::object tm = py::none(), py::object mm = py::none(), py::object ev = py::none(),
         py::object cn = py::none(), py::object sg = py::none(), py::object st = py::none(),
         py::object tp = py::none(), py::object pg = py::none(), py::object sp = py::none()) {
          TradeManagerPtr ctm;
          if (!tm.is_none()) {
              ctm = tm.cast<TradeManagerPtr>();
          }
          MoneyManagerPtr cmm;
          if (!mm.is_none()) {
              cmm = mm.cast<MoneyManagerPtr>();
          }
          EnvironmentPtr cev;
          if (!ev.is_none()) {
              cev = ev.cast<EnvironmentPtr>();
          }
          ConditionPtr ccn;
          if (!cn.is_none()) {
              ccn = cn.cast<ConditionPtr>();
          }
          SignalPtr csg;
          if (!sg.is_none()) {
              csg = sg.cast<SignalPtr>();
          }
          StoplossPtr cst;
          if (!st.is_none()) {
              cst = st.cast<StoplossPtr>();
          }
          StoplossPtr ctp;
          if (!tp.is_none()) {
              ctp = tp.cast<StoplossPtr>();
          }
          ProfitGoalPtr cpg;
          if (!pg.is_none()) {
              cpg = pg.cast<ProfitGoalPtr>();
          }
          SlippagePtr csp;
          if (!sp.is_none()) {
              csp = sp.cast<SlippagePtr>();
          }
          return SYS_Simple(ctm, cmm, cev, ccn, csg, cst, ctp, cpg, csp);
      },
      py::arg("tm") = py::none(), py::arg("mm") = py::none(), py::arg("ev") = py::none(),
      py::arg("cn") = py::none(), py::arg("sg") = py::none(), py::arg("st") = py::none(),
      py::arg("tp") = py::none(), py::arg("pg") = py::none(), py::arg("sp") = py::none(),
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

    m.def(
      "SYS_WalkForward",
      [](const py::sequence& candidate_sys_list, const TradeManagerPtr& tm, size_t train_len,
         size_t test_len, const SelectorPtr& se, const TradeManagerPtr& train_tm) {
          SystemList sys_list = python_list_to_vector<SystemPtr>(candidate_sys_list);
          SelectorPtr c_se = se;
          if (!c_se) {
              c_se = SE_PerformanceOptimal();
          }
          return SYS_WalkForward(sys_list, tm, train_len, test_len, c_se, train_tm);
      },
      py::arg("sys_list"), py::arg("tm") = TradeManagerPtr(), py::arg("train_len") = 100,
      py::arg("test_len") = 20, py::arg("se") = SelectorPtr(),
      py::arg("train_tm") = TradeManagerPtr(),
      R"(SYS_WalkForward(sys_list, tm, train_len, test_len, train_tm)

  创建滚动寻优系统，当输入的候选系统列表中仅有一个候选系统时，即为滚动系统

  :param sequence sys_list: 候选系统列表
  :param TradeManager tm: 交易账户
  :param int train_len: 滚动评估系统绩效时使用的数据长度
  :param int test_len: 使用在 train_len 中选出的最优系统执行的数据长度
  :param SelectorBase se: 寻优选择器，默认为按“帐户平均年收益率%”最大选择
  :param TradeManager train_tm: 滚动评估时使用的交易账户, 为None时, 使用 tm 的拷贝进行评估)");
}
