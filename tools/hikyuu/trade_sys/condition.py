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


from ._trade_sys import ConditionBase, CN_OPLine
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

ConditionBase.__unicode__ = unicodeFunc
ConditionBase.__repr__ = reprFunc


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


#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

ConditionBase.__doc__ = """
系统有效条件基类

自定义系统有效条件接口：

    ConditionBase._calculate() - 【必须】子类计算接口
    ConditionBase._clone() - 【必须】克隆接口
    ConditionBase._reset() - 【可选】重载私有变量
"""

ConditionBase.name.__doc__ = """名称"""

ConditionBase.__init__.__doc__ = """
__init__(self[, name="ConditionBase"])
    
    初始化构造函数
        
    :param str name: 名称
"""

ConditionBase.getParam.__doc__ = """
getParam(self, name)

    获取指定的参数
    
    :param str name: 参数名称
    :return: 参数值
    :raises out_of_range: 无此参数
"""

ConditionBase.setParam.__doc__ = """
setParam(self, name, value)
    
    设置参数
        
    :param str name: 参数名称
    :param value: 参数值
    :type value: int | bool | float | string
    :raises logic_error: Unsupported type! 不支持的参数类型
"""

ConditionBase.isValid.__doc__ = """
isValid(self, datetime)
    
    指定时间系统是否有效
        
    :param Datetime datetime: 指定时间
    :return: True 有效 | False 无效
"""

ConditionBase.setTO.__doc__ = """
setTO(self, k)
    
    :param KData k: 设置交易对象
"""

ConditionBase.getTO.__doc__ = """
getTO(self)
    
    :return: 交易对象
    :rtype: KData 
"""

ConditionBase.setTM.__doc__ = """
setTM(self, tm)
    
    :param TradeManager tm: 设置交易管理账户
"""

ConditionBase.getTM.__doc__ = """
getTM(self)
    
    获取交易管理账户
        
    :return: 交易管理账户
    :rtype: TradeManager 
"""

ConditionBase.setSG.__doc__ = """
setSG(self, sg)

    设置交易信号指示器
        
    :param SignalBase sg:
"""

ConditionBase.getSG.__doc__ = """
getSG(self)
    
    获取信号指示器
        
    :rtype: SignalBase
"""

ConditionBase.reset.__doc__ = """
reset(self, with_tm, with_ev, with_mm, with_sp)
    
    复位操作。TM、EV、MM、SP都是和具体系统无关的策略组件，可以在不同的系统中进行共享，
    复位将引起系统运行时被重新清空并计算。尤其是在共享TM时需要注意！
        
    :param bool with_tm: 是否复位TM组件
    :param bool with_ev: 是否复位EV组件
    :param bool with_mm: 是否复位MM组件
    :param bool with_sp: 是否复位SP组件
"""

ConditionBase.clone.__doc__ = """
clone(self, with_tm, with_ev, with_mm, with_sp)
    
    克隆操作。TM、EV、MM、SP都是和具体系统无关的策略组件，可以在不同的系统中进行共享。
    clone将生成新的独立实例，此时非共享状态。尤其需要注意TM是否共享的情况！
        
    :param bool with_tm: 是clone还是共享
    :param bool with_ev: 是clone还是共享
    :param bool with_mm: 是clone还是共享
    :param bool with_sp: 是clone还是共享
"""

ConditionBase._addValid.__doc__ = """
_addValid(self, datetime)
    
    加入有效时间，在_calculate中调用
        
    :param Datetime datetime: 有效时间
"""

ConditionBase._calculate.__doc__ = """
_calculate(self)
    
    【重载接口】子类计算接口
"""

ConditionBase._reset.__doc__ = """
_reset(self)
    
    【重载接口】子类复位接口，复位内部私有变量
"""

ConditionBase._clone.__doc__ = """
_clone(self)
    
    【重载接口】子类克隆接口
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

CN_OPLine.__doc__ = """
CN_OPLine(op)

    固定使用股票最小交易量进行交易，计算权益曲线的op值，当权益曲线高于op时，系统有效，否则无效。

    :param Operand op: Operand实例
    :return: 系统有效条件实例
    :rtype: ConditionBase
""" 
