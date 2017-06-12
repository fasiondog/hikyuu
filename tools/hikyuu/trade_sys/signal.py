#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from . import _trade_sys as csys
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

SignalBase = csys.SignalBase
SignalBase.__unicode__ = unicodeFunc
SignalBase.__repr__ = reprFunc

SG_Single = csys.SG_Single
SG_Single2 = csys.SG_Single2
SG_Cross = csys.SG_Cross
SG_CrossGold = csys.SG_CrossGold
SG_Flex = csys.SG_Flex

def sig_init(self, name, params):
    super(self.__class__, self).__init__(name)
    for k,v in params.items():
        self.setParam(k, v)
        
def sig_calculate(self, func):
    self._calculate = func
    
def crtSG(func, params={}, name='crtSG'):
    """
    快速创建自定义不带私有属性的信号指示器
    func  : 信号函数
    params: 参数字典，如 {'n': 20}
    name  : 信号指示器名称，默认"crtSG"
    """
    meta_x = type(name, (SignalBase,), {'__init__': sig_init})
    meta_x._clone = lambda name, params: meta_x(name, params)
    meta_x._calculate = func
    return meta_x(name, params)

SG_Flex.__doc__ = SG_Flex.__doc__ + """\n
使用指标的EMA平滑值作为慢线，但指标上穿慢线时买入，指标下穿慢线时卖出。
参数：
    n：慢线EMA参数
"""