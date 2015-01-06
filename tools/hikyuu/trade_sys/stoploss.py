#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

StoplossBase = csys.StoplossBase
StoplossBase.__unicode__ = unicodeFunc
StoplossBase.__repr__ = reprFunc

def FixedPercent_SL(p):
    """
    固定百分比止损策略
    p: (0, 1]
    """
    return csys.FixedPercent_SL(p)

def Saftyloss_ST(n1 = 10, n2 = 3, p = 2.0):
    return csys.Saftyloss_ST(n1, n2, p)