#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2010-2017 fasiondog
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


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


#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

StoplossBase.__doc__ = """
止损/止赢算法基类

自定义止损/止赢策略接口：

    SignalBase._calculate() - 【必须】子类计算接口
    SignalBase._clone() - 【必须】克隆接口
    SignalBase._reset() - 【可选】重载私有变量
"""

StoplossBase.name.__doc__ = """名称"""

StoplossBase.__init__.__doc__ = """
__init__(self[, name="StoplossBase"])
    
    :param str name: 名称
"""

StoplossBase.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

StoplossBase.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

StoplossBase.setTM.__doc__ = """
setTM(self, tm)
        
    设置交易管理实例
        
    :param TradeManager tm: 交易管理实例
"""

StoplossBase.getTM.__doc__ = """
getTM(self)
    
    获取交易管理实例
        
    :rtype: TradeManager
"""

StoplossBase.setTO.__doc__ = """
setTO(self, k)
    
    :param KData k: 设置交易对象
"""

StoplossBase.getTO.__doc__ = """
getTO(self)
    
    :return: 交易对象
    :rtype: KData
"""

StoplossBase.getPrice.__doc__ = """
getPrice(self, datetime, price)
    
    【重载接口】获取本次预期交易（买入）时的计划止损价格，如果不存在止损价，则返回0。
    用于系统在交易执行前向止损策略模块查询本次交易的计划止损价。
        
    .. note::
        一般情况下，止损/止赢的算法可以互换，但止损的getPrice可以传入计划交易的
        价格，比如以买入价格的30%做为止损。而止赢则不考虑传入的price参数，即认为
        price 为 0.0。实际上，即使止损也不建议使用price参数，如可以使用前日最低
        价的 30% 作为止损，则不需要考虑price参数。
        
    :param Datetime datetime: 交易时间
    :param float price: 计划买入的价格
    :return: 止损价格
    :rtype: float
"""

StoplossBase.reset.__doc__ = """
reset(self)
    
    复位操作
"""

StoplossBase.clone.__doc__ = """
clone(self)
    
    克隆操作
"""

StoplossBase._calculate.__doc__ = """
_calculate(self)
    
    【重载接口】子类计算接口
"""

StoplossBase._reset.__doc__ = """
_reset(self)
    
    【重载接口】子类复位接口，复位内部私有变量
"""

StoplossBase._clone.__doc__ = """
_clone(self)
    
    【重载接口】子类克隆接口
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

ST_FixedPercent.__doc__ =  """
ST_FixedPercent([p=0.03])

    固定百分比止损策略，即当价格低于买入价格的某一百分比时止损
    
    :param float p: 百分比(0,1]
    :return: 止损/止赢策略实例
"""

ST_Indicator.__doc__ = """
ST_Indicator(op[, kpart="CLOSE"])

    使用技术指标作为止损价。如使用10日EMA作为止损：::
    
        ST_Indicator(OP(EMA(n=10)))

    :param Operand op:
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: 止损/止赢策略实例
"""

ST_Saftyloss.__doc__ = """
ST_Saftyloss([n1=10, n2=3, p=2.0])

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