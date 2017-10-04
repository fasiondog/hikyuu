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


from ._trade_sys import (SlippageBase, SL_FixedPercent, SL_FixedValue)
from hikyuu.util.unicode import (unicodeFunc, reprFunc)


SlippageBase.__unicode__ = unicodeFunc
SlippageBase.__repr__ = reprFunc


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
  

#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

SlippageBase.__doc__ = """
移滑价差算法基类

自定义移滑价差接口：

    SlippageBase.getRealBuyPrice() - 【必须】计算实际买入价格
    SlippageBase.getRealSellPrice() - 【必须】计算实际卖出价格
    SlippageBase._calculate() - 【必须】子类计算接口
    SlippageBase._clone() - 【必须】克隆接口
    SlippageBase._reset() - 【可选】重载私有变量
"""

SlippageBase.name.__doc__ = """名称"""

SlippageBase.__init__.__doc__ = """
__init__(self[, name="SlippageBase"])
    
    初始化构造函数
        
    :param str name: 名称
"""

SlippageBase.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

SlippageBase.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

SlippageBase.setTO.__doc__ = """
setTO(self, k)
    
    :param KData k: 设置交易对象
"""

SlippageBase.getTO.__doc__ = """
getTO(self)
    
    :return: 交易对象
    :rtype: KData
"""

SlippageBase.getRealBuyPrice.__doc__ = """
getRealBuyPrice(self, datetime, price)

    【重载接口】计算实际买入价格
        
    :param Datetime datetime: 买入时间
    :param float price: 计划买入价格
    :return: 实际买入价格
    :rtype: float
"""

SlippageBase.getRealSellPrice.__doc__ = """
getRealSellPrice(self, datetime, price)

    【重载接口】计算实际卖出价格
        
    :param Datetime datetime: 卖出时间
    :param float price: 计划卖出价格
    :return: 实际卖出价格
    :rtype: float        
"""

SlippageBase.reset.__doc__ = """
reset(self)
    
    复位操作
"""

SlippageBase.clone.__doc__ = """
clone(self)
    
    克隆操作    
"""

SlippageBase._calculate.__doc__ = """
_calculate(self)
    
    【重载接口】子类计算接口
"""

SlippageBase._reset.__doc__ = """
_reset(self)
    
    【重载接口】子类复位接口，复位内部私有变量
"""

SlippageBase._clone.__doc__ = """
_clone(self)
    
    【重载接口】子类克隆接口  
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

SL_FixedPercent.__doc__ = """
SP_FixedPercent([p=0.001])

    固定百分比移滑价差算法，买入实际价格 = 计划买入价格 * (1 + p)，卖出实际价格 = 计划卖出价格 * (1 - p)
    
    :param float p: 偏移的固定百分比
    :return: 移滑价差算法实例
"""

SL_FixedValue.__doc__ = """
SP_FixedValuet([p=0.001])

    固定价格移滑价差算法，买入实际价格 = 计划买入价格 + 偏移价格，卖出实际价格 = 计划卖出价格 - 偏移价格
    
    :param float p: 偏移价格
    :return: 移滑价差算法实例
"""