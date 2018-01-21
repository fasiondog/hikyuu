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


from ._trade_sys import (Portfolio, PF_Simple)
from hikyuu.util.unicode import (unicodeFunc, reprFunc)

Portfolio.__unicode__ = unicodeFunc
Portfolio.__repr__ = reprFunc


#------------------------------------------------------------------
# add doc-string
#------------------------------------------------------------------

Portfolio.__doc__ = """实现多标的、多策略的投资组合"""

Portfolio.name.__doc__ = """名称"""

Portfolio.tm.__doc__ = """关联的交易管理实例"""

Portfolio.se.__doc__ = """选择器策略"""

Portfolio.run.__doc__ = """
run(self, query)
    
    运行投资组合策略
        
    :param Query query: 查询条件
"""


#------------------------------------------------------------------
# add doc-string for build_in func
#------------------------------------------------------------------

PF_Simple.__doc__ = """
PF_Simple([tm, sys, se])

    创建一个多标的、单系统策略的投资组合
    
    :param TradeManager tm: 交易管理
    :param System sys: 系统策略
    :param SelectorBase se: 选择器
"""

