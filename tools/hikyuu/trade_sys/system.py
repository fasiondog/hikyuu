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
    return csys.SYS_Simple(tm, mm, ev, cn, sg, sl, tp, pg, sp);