#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from ._trade_sys import (System,
                         SystemList,
                         getSystemPartName,
                         getSystemPartEnum,
                         TradeRequest)
from ._trade_sys import SYS_Simple as cpp_SYS_Simple
from hikyuu.util.unicode import (unicodeFunc, reprFunc)
from hikyuu.util.slice import list_getitem
    
System.__unicode__ = unicodeFunc
System.__repr__ = reprFunc

System.ENVIRONMENT = System.Part.ENVIRONMENT
System.CONDITION = System.Part.CONDITION
System.SIGNAL = System.Part.SIGNAL
System.STOPLOSS = System.Part.STOPLOSS
System.TAKEPROFIT = System.Part.TAKEPROFIT
System.MONEYMANAGER = System.Part.MONEYMANAGER
System.PROFITGOAL = System.Part.PROFITGOAL
System.SLIPPAGE = System.Part.SLIPPAGE
System.INVALID = System.Part.INVALID

SystemList.__getitem__ = list_getitem

#SystemPart = System.Part


def SYS_Simple(tm = None, mm = None, ev = None, cn = None, 
               sg = None, st = None, tp = None, pg = None, sp = None):
    """
    创建简单系统实例（每次交易不进行多次加仓或减仓，即每次买入后在卖出时全部卖出）， 
     系统实例在运行时(调用run方法），至少需要一个配套的交易管理实例、一个资金管理策略
    和一个信号指示器），可以在创建系统实例后进行指定。如果出现调用run时没有任何输出，
    且没有正确结果的时候，可能是未设置tm、sg、mm。进行回测时，使用 run 方法，如::
    
        #创建模拟交易账户进行回测，初始资金30万
        my_tm = crtTM(initCash = 300000)

        #创建信号指示器（以5日EMA为快线，5日EMA自身的10日EMA最为慢线，快线向上穿越
        #慢线时买入，反之卖出）
        my_sg = SG_Flex(OP(EMA(n=5)), slow_n=10)

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
    :return: system实例    
    """
    sys_ins = cpp_SYS_Simple()
    if tm:
        sys_ins.tm = tm
    if mm:
        sys_ins.mm = mm
    if ev:
        sys_ins.ev = ev
    if cn:
        sys_ins.cn = cn
    if sg:
        sys_ins.sg = sg
    if st:
        sys_ins.st = st
    if tp:
        sys_ins.tp = tp
    if pg:
        sys_ins.pg = pg
    if sp:
        sys_ins.sp = sp
    return sys_ins


#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

TradeRequest.__doc__ = """
交易请求记录。系统内部在实现延迟操作时登记的交易请求信息。暴露该结构的主要目的是用于
在“delay”模式（延迟到下一个bar开盘时进行交易）的情况下，系统实际已知下一个Bar将要
进行交易，此时可通过 System.getBuyTradeRequest() 、 System.getSellTradeRequest()
来获知下一个BAR是否需要买入/卖出。主要用于提醒或打印下一个Bar需要进行操作。对于系统
本身的运行没有影响。
"""

TradeRequest.valid.__doc__ = """该交易请求记录是否有效（True | False）"""
TradeRequest.business.__doc__ = """
交易业务类型，参见：:py:class:`hikyuu.trade_manage.BUSINESS`
"""
TradeRequest.datetime.__doc__ = """发出交易请求的时刻"""
TradeRequest.stoploss.__doc__ = """发出交易请求时刻的止损价"""
TradeRequest.part.__doc__ = """发出交易请求的来源，参见：:py:class:`System.Part`"""
TradeRequest.count.__doc__ = """因操作失败，连续延迟的次数"""



System.Part.__doc__ = """
系统部件枚举值，系统的买入/卖出等操作可由这些部件触发，用于标识实际交易指令的来源，
参见：:py:class:`TradeRecord`。

实际使用中，可使用 System.ENVIRONMENT 的简化方式 代替 System.Part.ENVIRONMENT，
其他与此类似。

- System.Part.ENVIRONMENT  - 市场环境判断策略
- System.Part.CONDITION    - 系统有效条件
- System.Part.SIGNAL       - 信号指示器
- System.Part.STOPLOSS     - 止损策略
- System.Part.TAKEPROFIT   - 止盈策略
- System.Part.MONEYMANAGER - 资金管理策略
- System.Part.PROFITGOAL   - 盈利目标策略
- System.Part.SLIPPAGE     - 移滑价差算法
- System.Part.INVALID      - 无效值边界，大于等于该值时为无效部件
"""


System.__doc__ = """
系统基类。需要扩展或实现更复杂的系统交易行为，可从此类继承。

系统是指针对针对单个交易对象的完整策略，包括环境判断、系统有效条件、资金管理、止损、
止盈、盈利目标、移滑价差的完整策略，用于模拟回测。

公共参数：

    delay=True (bool) : 是否延迟到下一个bar开盘时进行交易
    delay_use_current_price=True (bool) : 延迟操作的情况下，是使用当前交易时bar
                                          的价格计算新的止损价/止赢价/目标价还是
                                          使用上次计算的结果
    max_delay_count=3 (int) : 连续延迟交易请求的限制次数
    tp_monotonic=True (bool) : 止赢单调递增
    tp_delay_n=3 (int) : 止盈延迟开始的天数，即止盈策略判断从实际交易几天后开始生效
    ignore_sell_sg=False (bool) : 忽略卖出信号，只使用止损/止赢等其他方式卖出
    support_borrow_cash=False (bool) : 在现金不足时，是否支持借入现金，融资
    support_borrow_stock=False (bool) : 在没有持仓时，是否支持借入证券，融券
"""

System.name.__doc__ = """系统名称""" 
System.tm.__doc__ = """交易管理实例"""
System.mm.__doc__ = """资金管理策略"""
System.ev.__doc__ = """市场环境判断策略"""
System.cn.__doc__ = """系统有效条件"""
System.sg.__doc__ = """信号指示器"""
System.st.__doc__ = """止损策略"""
System.tp.__doc__ = """止盈策略"""
System.pg.__doc__ = """盈利目标策略"""
System.sp.__doc__ = """移滑价差算法"""


System.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

System.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

System.getTO.__doc__ = """
getTO(self)

    获取交易对象，即运行 :py:meth:`System.run` 方法时，stock + query 获取到的 KData
        
    :rtype: KData
"""

System.setTO.__doc__ = """
setTO(self, kdata)

    设置交易对象，即运行 :py:meth:`System.run` 方法时，stock + query 获取到的 KData
        
    :param KData kdata: K线数据
"""

System.getStock.__doc__ = """
getStock(self)
    
    获取关联的证券
        
    :rtype: Stock
"""

System.getTradeRecordList.__doc__ = """
getTradeRecordList(self)
    
    获取交易记录
        
    :rtype: TradeRecordList
"""

System.getBuyTradeRequest.__doc__ = """
getBuyTradeRequest(self)
    
    获取买入请求，“delay”模式下查看下一时刻是否存在买入操作
        
    :rtype: TradeRequest
"""

System.getSellTradeRequest.__doc__ = """
getSellTradeRequest(self)
    
    获取卖出请求，“delay”模式下查看下一时刻是否存在卖出操作
        
    :rtype: TradeRequest
"""

System.run.__doc__ = """
run(self, stock, query[, reset=True])
    
    运行系统，执行回测
        
    :param Stock stock: 交易的证券
    :param Query query: K线数据查询条件
    :param bool reset: 是否同时复位所有组件，尤其是tm实例
"""

System.reset.__doc__ = """
reset(self, with_tm, with_ev)
    
    复位操作。TM、EV是和具体系统无关的策略组件，可以在不同的系统中进行共享，
    复位将引起系统运行时被重新清空并计算。尤其是在共享TM时需要注意！
        
    :param bool with_tm: 是否复位TM组件
    :param bool with_ev: 是否复位EV组件
"""

System.clone.__doc__ = """
clone(self, with_tm, with_ev)
    
    克隆操作。T
    M、EV是和具体系统无关的策略组件，可以在不同的系统中进行共享。clone将生成新的
    独立实例，此时非共享状态。尤其需要注意TM是否共享的情况！
        
    :param bool with_tm: 是clone还是共享
    :param bool with_ev: 是clone还是共享
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

getSystemPartName.__doc__ = """
getSystemPartName(part)

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
    :rtype: str
"""

getSystemPartEnum.__doc__ = """
getSystemPartEnum(part_name)

     根据系统部件的字符串名称获取相应的枚举值

    :param str part_name: 系统部件的字符串名称，参见：:py:func:`getSystemPartName`
    :rtype: System.Part
"""
