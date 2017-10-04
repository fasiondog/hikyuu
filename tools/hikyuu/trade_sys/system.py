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
                         getSystemPartName,
                         getSystemPartEnum,
                         TradeRequest)
from ._trade_sys import SYS_Simple as cpp_SYS_Simple
from hikyuu.util.unicode import (unicodeFunc, reprFunc)
    
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

#SystemPart = System.Part


def SYS_Simple(tm = None, mm = None, ev = None, cn = None, 
               sg = None, sl = None, tp = None, pg = None, sp = None):
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
    :param StoplossBase sl: 止损策略
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
    if sl:
        sys_ins.sl = sl
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

System.run.__doc__ = """
run(self, stock, query[, reset=True])
    
    运行系统，执行回测
        
    :param Stock stock: 交易的证券
    :param Query query: K线数据查询条件
    :param bool reset: 是否同时复位所有组件，尤其是tm实例
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
        - System.Part.STOPLOSS     - "SL"
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
