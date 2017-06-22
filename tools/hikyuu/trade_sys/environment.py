#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

EnvironmentBase = csys.EnvironmentBase
EnvironmentBase.__unicode__ = unicodeFunc
EnvironmentBase.__repr__ = reprFunc

EV_TwoLine = csys.EV_TwoLine

def ev_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k,v in params.items():
        self.setParam(k, v)
    
def crtEV(func, params={}, name='crtEV'):
    """
    快速创建自定义不带私有属性的市场环境判断策略
    
    :param func: 市场环境判断策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义市场环境判断策略实例
    """
    meta_x = type(name, (EnvironmentBase,), {'__init__': ev_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)


EV_TwoLine.__doc__ += """\n
    快慢线判断策略，市场指数的快线大于慢线时，市场有效，否则无效。

    :param Operand fast: 快线指标
    :param Operand slow: 慢线指标
    :param string market: 市场名称
"""