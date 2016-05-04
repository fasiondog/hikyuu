#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from ._trade_sys import *
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

MoneyManagerBase.__unicode__ = unicodeFunc
MoneyManagerBase.__repr__ = reprFunc

MM_FixedCount.__doc__ = """
    固定交易数量资金管理策略 
    n: 固定交易的数量，为大于等于1的整数，如小于1,默认被置为0
    """

MM_FixedPercent.__doc__ = """
百分比风险模型
公式：P（头寸规模）＝ C（总风险）/ R（每股的风险） ［这里C现金为总风险］
参数：
p 每笔交易总风险占总资产的百分比，如0.02表示总资产的2%
"""

