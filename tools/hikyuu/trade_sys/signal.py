#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20130419, Added by fasiondog
#===============================================================================

from ._trade_sys import *
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

SignalBase.__unicode__ = unicodeFunc
SignalBase.__repr__ = reprFunc

def sig_init(self, name, params):
    super(self.__class__, self).__init__(name)
    for k,v in params.items():
        self.setParam(k, v)
        
def sig_calculate(self, func):
    self._calculate = func
    
def crtSig(func, params={}, name='crtSig',):
    meta_x = type(name, (SignalBase,), {'__init__': sig_init})
    meta_x._clone = lambda name, params: meta_x(name, params)
    meta_x._calculate = func
    return meta_x(name, params)

SG_Flex.__doc__ = SG_Flex.__doc__ + """\n
使用指标的EMA平滑值作为慢线，但指标上穿慢线时买入，指标下穿慢线时卖出。
参数：
    n：慢线EMA参数
"""