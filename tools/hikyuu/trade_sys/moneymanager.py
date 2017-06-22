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

MM_FixedRisk = csys.MM_FixedRisk
MM_FixedCapital = csys.MM_FixedCapital
MM_FixedCount = csys.MM_FixedCount
MM_FixedPercent = csys.MM_FixedPercent
MM_FixedUnits = csys.MM_FixedUnits
MM_FixedCapital = csys.MM_FixedCapital
MM_WilliamsFixedRisk = csys.MM_WilliamsFixedRisk
MM_Nothing = csys.MM_Nothing

def mm_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k,v in params.items():
        self.setParam(k, v)
    
def crtMM(func, params={}, name='crtMM'):
    """
    快速创建自定义不带私有属性的资金管理策略
    
    :param func: 资金管理策略计算函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义资金管理策略实例
    """
    meta_x = type(name, (MoneyManagerBase,), {'__init__': mm_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)

MM_Nothing.__doc__ += """\n
    特殊的资金管理策略，相当于不做资金管理，有多少钱买多少。
"""

MM_FixedCount.__doc__ += """\n
    固定交易数量资金管理策略。每次买入固定的数量。
    
    :param int n: 每次买入的数量（应该是交易对象最小交易数量的整数，此处程序没有此进行判断）
    :return: 资金管理策略实例
"""

MM_FixedPercent.__doc__ += """\n
    固定百分比风险模型。公式：P（头寸规模）＝ 账户余额 * 百分比 / R（每股的交易风险）。[BOOK3]_, [BOOK4]_ .
    
    :param float p: 百分比
    :return: 资金管理策略实例
"""

