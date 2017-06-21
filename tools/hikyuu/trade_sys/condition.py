#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

ConditionBase = csys.ConditionBase
ConditionBase.__unicode__ = unicodeFunc
ConditionBase.__repr__ = reprFunc

CN_OPLine = csys.CN_OPLine

CN_OPLine.__doc__ += """\n
    固定使用股票最小交易量进行交易，计算权益曲线的op值，当权益曲线高于op时，系统有效，否则无效。

    :param Operand op: Operand实例
    :return: 系统有效条件实例
""" 

def cn_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k,v in params.items():
        self.setParam(k, v)
    
def crtCN(func, params={}, name='crtCN'):
    """
    快速创建自定义不带私有属性的系统有效条件
    
    :param func: 系统有效条件函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义系统有效条件实例
    """
    meta_x = type(name, (ConditionBase,), {'__init__': cn_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)


