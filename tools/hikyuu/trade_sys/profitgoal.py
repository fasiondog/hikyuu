#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

ProfitGoalBase = csys.ProfitGoalBase
ProfitGoalBase.__unicode__ = unicodeFunc
ProfitGoalBase.__repr__ = reprFunc

PG_NoGoal = csys.PG_NoGoal
PG_FixedPercent = csys.PG_FixedPercent

PG_NoGoal.__doc__ += """\n
    无盈利目标策略，通常为了进行测试或对比。
    
    :return: 盈利目标策略实例
"""

PG_FixedPercent.__doc__ += """\n
    固定百分比盈利目标，目标价格 = 买入价格 * (1 + p)
    
    :param float p: 百分比
    :return: 盈利目标策略实例
"""

def pg_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k,v in params.items():
        self.setParam(k, v)
    
def crtPG(func, params={}, name='crtPG'):
    """
    快速创建自定义不带私有属性的盈利目标策略
    
    :param func: 盈利目标策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 盈利目标策略实例
    """
    meta_x = type(name, (ProfitGoalBase,), {'__init__': pg_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)