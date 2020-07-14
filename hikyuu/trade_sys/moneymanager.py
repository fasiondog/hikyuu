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

from hikyuu.cpp.core import (
    MoneyManagerBase, MM_Nothing, MM_FixedRisk, MM_FixedCapital, MM_FixedCount, MM_FixedPercent,
    MM_FixedUnits, MM_WilliamsFixedRisk
)


def mm_init(self, name, params):
    super(self.__class__, self).__init__(name)
    self._name = name
    self._params = params
    for k, v in params.items():
        self.setParam(k, v)


def crtMM(func, params={}, name='crtMM'):
    """
    快速创建自定义不带私有属性的资金管理策略
    
    :param func: 资金管理策略计算函数
    :param {} params: 参数字典
    :param str name: 自定义名称
    :return: 自定义资金管理策略实例
    """
    meta_x = type(name, (MoneyManagerBase, ), {'__init__': mm_init})
    meta_x._clone = lambda self: meta_x(self._name, self._params)
    meta_x._calculate = func
    return meta_x(name, params)


#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

MoneyManagerBase.__init__.__doc__ = """
__init__(self[, name="MoneyManagerBase])
    
    初始化构造函数
        
    :param str name: 名称
"""
