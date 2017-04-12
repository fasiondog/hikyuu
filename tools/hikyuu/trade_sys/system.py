#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

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