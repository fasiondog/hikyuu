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


from ._trade_sys import EnvironmentBase, EV_TwoLine
from hikyuu.util.unicode import (unicodeFunc, reprFunc)


EnvironmentBase.__unicode__ = unicodeFunc
EnvironmentBase.__repr__ = reprFunc


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



#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

EnvironmentBase.__doc__ = """
市场环境判定策略基类

自定义市场环境判定策略接口：

    EnvironmentBase._calculate() - 【必须】子类计算接口
    EnvironmentBase._clone() - 【必须】克隆接口
    EnvironmentBase._reset() - 【可选】重载私有变量
"""

EnvironmentBase.__init__.__doc__ = """
__init__(self[, name='EnvironmentBase'])
    
    初始化构造函数
        
    :param str name: 名称
"""

EnvironmentBase.name.__doc__ = """名称"""

EnvironmentBase.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

EnvironmentBase.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

EnvironmentBase.setQuery.__doc__ = """
setQuery(self, query)
    
    设置查询条件
    
    :param KQuery query:  查询条件
"""

EnvironmentBase.getQuery.__doc__ = """
getQuery(self)
    
    获取查询条件
        
    :return: 查询条件
    :rtype: KQuery 
"""

EnvironmentBase.isValid.__doc__ = """
isValid(self, datetime)
    
    指定时间系统是否有效
        
    :param Datetime datetime: 指定时间
    :return: True 有效 | False 无效
"""

EnvironmentBase._addValid.__doc__ = """
_addValid(self, datetime)
    
    加入有效时间，在_calculate中调用
        
    :param Datetime datetime: 有效时间
"""

EnvironmentBase.reset.__doc__ = """
reset(self)
    
    复位操作
"""

EnvironmentBase.clone.__doc__ = """
clone(self)
    
    克隆操作
"""

EnvironmentBase._reset.__doc__ = """
_reset(self)
    
    【重载接口】子类复位接口，用于复位内部私有变量
"""

EnvironmentBase._clone.__doc__ = """
_clone(self)
    
    【重载接口】子类克隆接口
"""

EnvironmentBase._calculate.__doc__ = """
_calculate(self)
    
    【重载接口】子类计算接口
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

EV_TwoLine.__doc__ = """
EV_TwoLine(fast, slow[, market = 'SH'])

    快慢线判断策略，市场指数的快线大于慢线时，市场有效，否则无效。

    :param Operand fast: 快线指标
    :param Operand slow: 慢线指标
    :param string market: 市场名称
"""