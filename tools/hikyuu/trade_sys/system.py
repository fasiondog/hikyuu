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


from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)
    
System = csys.System
System.__unicode__ = unicodeFunc
System.__repr__ = reprFunc

System.ENVIRONMENT = System.SystemPart.ENVIRONMENT
System.CONDITION = System.SystemPart.CONDITION
System.SIGNAL = System.SystemPart.SIGNAL
System.STOPLOSS = System.SystemPart.STOPLOSS
System.TAKEPROFIT = System.SystemPart.TAKEPROFIT
System.MONEYMANAGER = System.SystemPart.MONEYMANAGER
System.PROFITGOAL = System.SystemPart.PROFITGOAL
System.SLIPPAGE = System.SystemPart.SLIPPAGE
System.INVALID = System.SystemPart.INVALID

SystemPart = System.SystemPart

getSystemPartName = csys.getSystemPartName
getSystemPartEum = csys.getSystemPartEnum


def SYS_Simple(tm = None, mm = None, ev = None, cn = None, 
               sg = None, sl = None, tp = None, pg = None, sp = None):
    """
    创建简单系统实例（每次交易不进行多次加仓或减仓，即每次买入后在卖出时全部卖出），
    系统实例在运行时(调用run方法），至少需要一个配套的交易管理实例、一个资金管理策略
    和一个信号指示器），可以在创建系统实例后进行指定。如果出现调用run时没有任何输出，
    且没有正确结果的时候，可能是未设置tm、sg、mm。
    tm: 交易管理实例 
    mm: 资金管理策略
    ev: 市场环境判断策略
    cn: 系统有效条件
    sg: 信号指示器
    sl: 止损策略
    tp: 止盈策略
    pg: 盈利目标策略
    sp: 移滑价差算法
    """
    sys_ins = csys.SYS_Simple()
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