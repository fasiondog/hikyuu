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


from ._trade_sys import (SelectorBase, SE_Fixed)
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

SelectorBase.__unicode__ = unicodeFunc
SelectorBase.__repr__ = reprFunc


#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

SelectorBase.__doc__ = """
选择器策略基类，实现标的、系统策略的评估和选取算法

自定义选择器策略接口：

    SelectorBase.getSelectedStockList - 【必须】获取指定时刻选择的标的列表
    SelectorBase._reset - 【可选】重置私有属性
    SelectorBase._clone - 【必须】克隆接口
"""

SelectorBase.name.__doc__ = """名称"""

SelectorBase.__init__.__doc__ = """
__init__(self[, name="SelectorBase])
    
    初始化构造函数
        
    :param str name: 名称
"""

SelectorBase.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
        
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

SelectorBase.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型 
"""

SelectorBase.addStock.__doc__ = """
addStock(self, stock)

    加入初始标的
        
    :param Stock stock: 加入的初始标的
"""

SelectorBase.addStockList.__doc__ = """
addStockList(self, stock_list)
    
    加入初始标的列表
        
    :param StockList stock: 加入的初始标的列表
"""

SelectorBase.clear.__doc__ = """
clear(self)
    
    清除已加入的所有交易系统实例
"""


SelectorBase.getSelectedSystemList.__doc__ = """
getSelectedSystemList(self, datetime)
    
    【重载接口】获取指定时刻选取的标的
        
    :param Datetime datetime: 指定时刻
    :return: 选取的系统交易实例
    :rtype: SystemList
"""

SelectorBase._reset.__doc__ = """
_reset(self)
    
    【重载接口】子类复位接口，复位内部私有变量
"""

SelectorBase._clone.__doc__ = """
_clone(self)
    
    【重载接口】子类克隆接口 
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

SE_Fixed.__doc__ = """
SE_Fixed([stocklist])

    固定选择器，即始终选择初始划定的标的范围
    
    :param StockList stocklist: 初始划定的标的
    :return: SE选择器实例
"""
