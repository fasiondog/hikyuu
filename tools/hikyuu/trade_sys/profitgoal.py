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


from ._trade_sys import (ProfitGoalBase, 
                         PG_NoGoal, 
                         PG_FixedPercent,
                         PG_FixedHoldDays)
from hikyuu.util.unicode import (unicodeFunc, reprFunc)


ProfitGoalBase.__unicode__ = unicodeFunc
ProfitGoalBase.__repr__ = reprFunc


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


#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

ProfitGoalBase.__doc__ = """
盈利目标策略基类

自定义盈利目标策略接口：

    ProfitGoalBase.getGoal() - 【必须】获取目标价格
    ProfitGoalBase._calculate() - 【必须】子类计算接口
    ProfitGoalBase._clone() - 【必须】克隆接口
    ProfitGoalBase._reset() - 【可选】重载私有变量
"""

ProfitGoalBase.name.__doc__ = """名称"""

ProfitGoalBase.__init__.__doc__ = """
__init__(self[, name="ProfitGoalBase"])
    
    初始化构造函数
        
    :param str name: 名称
"""

ProfitGoalBase.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

ProfitGoalBase.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

ProfitGoalBase.setTO.__doc__ = """
setTO(self, k)
    
    :param KData k: 设置交易对象
"""

ProfitGoalBase.getTO.__doc__ = """
getTO(self)
    
    :return: 交易对象
    :rtype: KData
"""

ProfitGoalBase.setTM.__doc__ = """
setTM(self, tm)
    
    :param TradeManager tm: 设置交易管理账户
"""

ProfitGoalBase.getTM.__doc__ = """
getTM(self)
    
    获取交易管理账户
        
    :rtype: TradeManager
"""

ProfitGoalBase.getGoal.__doc__ = """
getGoal(self, datetime, price)
    
    【重载接口】获取盈利目标价格，返回constant.null_price时，表示未限定目标；
    返回0意味着需要卖出。
        
    :param Datetime datetime: 买入时间
    :param float price: 买入价格
    :return: 目标价格
    :rtype: float
"""

ProfitGoalBase.reset.__doc__ = """
reset(self)
    
    复位操作
"""

ProfitGoalBase.clone.__doc__ = """
clone(self)
    
    克隆操作
"""

ProfitGoalBase._calculate.__doc__ = """
_calculate(self)
    
    【重载接口】子类计算接口
"""

ProfitGoalBase._reset.__doc__ = """
_reset(self)
    
    【重载接口】子类复位接口，复位内部私有变量
"""

ProfitGoalBase._clone.__doc__ = """
_clone(self)
    
    【重载接口】子类克隆接口
"""

ProfitGoalBase.buyNotify.__doc__ = """
buyNotify(self, trade_record)
    
    【重载接口】交易系统发生实际买入操作时，通知交易变化情况，一般存在多次增减仓的情况才需要重载
        
    :param TradeRecord trade_record: 发生实际买入时的实际买入交易记录
"""

ProfitGoalBase.sellNotify.__doc__ = """
sellNotify(self, trade_record)
    
    【重载接口】交易系统发生实际卖出操作时，通知实际交易变化情况，一般存在多次增减仓的情况才需要重载
        
    :param TradeRecord trade_record: 发生实际卖出时的实际卖出交易记录
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

PG_NoGoal.__doc__ = """
PG_NoGoal()

    无盈利目标策略，通常为了进行测试或对比。
    
    :return: 盈利目标策略实例
"""

PG_FixedPercent.__doc__ = """
PG_FixedPercent([p = 0.2])

    固定百分比盈利目标，目标价格 = 买入价格 * (1 + p)
    
    :param float p: 百分比
    :return: 盈利目标策略实例
"""

PG_FixedHoldDays.__doc__ = """
PG_FixedHoldDays([days=5])

    固定持仓天数盈利目标策略
    
    :param int days: 允许持仓天数（按交易日算）,默认5天
    :return: 盈利目标策略实例
"""

