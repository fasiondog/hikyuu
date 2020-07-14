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

from hikyuu.cpp.core import (System, SystemList, getSystemPartName, getSystemPartEnum, TradeRequest)
from hikyuu.cpp.core import SYS_Simple as cpp_SYS_Simple
from hikyuu.util.slice import list_getitem

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


def SYS_Simple(tm=None, mm=None, ev=None, cn=None, sg=None, st=None, tp=None, pg=None, sp=None):
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
