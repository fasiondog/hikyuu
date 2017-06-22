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

ST_FixedPercent = csys.ST_FixedPercent
ST_Indicator = csys.ST_Indicator
ST_Saftyloss = csys.ST_Saftyloss

def st_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k,v in params.items():
        self.setParam(k, v)
    
def crtST(func, params={}, name='crtST'):
    """
    快速创建自定义不带私有属性的止损/止盈策略
    
    :param func: 止损/止盈策略函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 止损/止盈策略实例
    """
    meta_x = type(name, (StoplossBase,), {'__init__': st_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)

ST_FixedPercent.__doc__ +=  """\n
    固定百分比止损策略，即当价格低于买入价格的某一百分比时止损
    
    :param float p: 百分比(0,1]
    :return: 止损/止赢策略实例
"""

ST_Indicator.__doc__ += """\n
    使用技术指标作为止损价。如使用10日EMA作为止损：::
    
        ST_Indicator(OP(EMA(n=10)))

    :param Operand op:
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
"""

ST_Saftyloss.__doc__ += """\n
    参见《走进我的交易室》（2007年 地震出版社） 亚历山大.艾尔德(Alexander Elder) P202
    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，
    得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日
    最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的
    上移，在上述结果的基础上再取起N日（一般为3天）内的最高值

    :param int n1: 计算平均噪音的回溯时间窗口，默认为10天
    :param int n2: 对初步止损线去n2日内的最高值，默认为3
    :param double p: 噪音系数，默认为2
    :return: 止损/止赢策略实例
"""