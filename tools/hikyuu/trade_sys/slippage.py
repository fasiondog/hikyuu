#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

SlippageBase = csys.SlippageBase
SlippageBase.__unicode__ = unicodeFunc
SlippageBase.__repr__ = reprFunc

SL_FixedPercent = csys.SL_FixedPercent
SL_FixedValue = csys.SL_FixedValue

def sl_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k,v in params.items():
        self.setParam(k, v)
    
def crtSL(func, params={}, name='crtSL'):
    """
    快速创建自定义不带私有属性的移滑价差算法
    
    :param func: 移滑价差算法函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 移滑价差算法实例
    """
    meta_x = type(name, (SlippageBase,), {'__init__': sl_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)    
  

SL_FixedPercent.__doc__ += """\n
    固定百分比移滑价差算法，买入实际价格 = 计划买入价格 * (1 + p)，卖出实际价格 = 计划卖出价格 * (1 - p)
    
    :param float p: 偏移的固定百分比
    :return: 移滑价差算法实例
"""

SL_FixedValue.__doc__ += """\n
    固定价格移滑价差算法，买入实际价格 = 计划买入价格 + 偏移价格，卖出实际价格 = 计划卖出价格 - 偏移价格
    
    :param float p: 偏移价格
    :return: 移滑价差算法实例
"""