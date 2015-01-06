#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

MoneyManagerBase = csys.MoneyManagerBase
MoneyManagerBase.__unicode__ = unicodeFunc
MoneyManagerBase.__repr__ = reprFunc

def FixedCount_MM(n):
    """
    固定交易数量资金管理策略
    n: 固定交易的数量，为大于等于1的整数，如小于1,默认被置为0
    """
    return csys.FixedCount_MM(n)