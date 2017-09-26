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


from .core import *


#------------------------------------------------------------------
# from _DataType.cpp
#------------------------------------------------------------------

DatetimeList.__doc__ = """日期序列，对应C++中的std::vector<Datetime>"""


PriceList.__doc__ ="""价格序列，其中价格使用double表示，对应C++中的std::vector<double>。"""


toPriceList.__doc__ += """\n
将Python的可迭代对象如 list、tuple 转化为 PriceList
    
    :param arg: 待转化的Python序列
    :rtype: PriceList
"""


StringList.__doc__ = """字符串列表，对应C++中的std::vector<String>"""



#------------------------------------------------------------------
# from _Datetime.cpp
#------------------------------------------------------------------

Datetime.__doc__ = """日期时间类，精确到秒"""

Datetime.max.__doc__ += """
获取支持的最大日期
"""

Datetime.min.__doc__ += """
获取支持的最小日期"""

Datetime.now.__doc__ += """
获取当前日期时间"""


getDateRange.__doc__ += """\n
获取指定 [start, end) 日期时间范围的自然日日历日期列表，仅支持到日

:param Datetime start: 起始日期
:param Datetime end: 结束日期
:rtype: DatetimeList
"""


#------------------------------------------------------------------
# from _util.cpp
#------------------------------------------------------------------

roundUp.__doc__ = """
roundUp(arg1[, arg2=0])

    向上截取，如10.1截取后为11
    
    :param float arg1: 待处理数据
    :param int arg2: 保留小数位数
    :return: 处理过的数据
"""

roundDown.__doc__ = """
roundDown(arg1[, arg2=0])

    向下截取，如10.1截取后为10
    
    :param float arg1: 待处理数据
    :param int arg2: 保留小数位数
    :return: 处理过的数据
"""


#------------------------------------------------------------------
# from _MarketInfo.cpp
#------------------------------------------------------------------

MarketInfo.__doc__ = """
    市场信息记录
    
    .. py:attribute:: market : 市场简称（如：沪市“SH”, 深市“SZ”）
    .. py:attribute:: name : 市场全称
    .. py:attribute:: description :描述说明
    .. py:attribute:: code : 该市场对应的主要指数，用于获取交易日历
    .. py:attribute:: lastDate : 该市场K线数据最后交易日期
"""


#------------------------------------------------------------------
# from _StockTypeInfo.cpp
#------------------------------------------------------------------

StockTypeInfo.__doc__ = """
    股票类型详情记录
    
    .. py:attribute:: type : 证券类型
    .. py:attribute:: description : 描述信息
    .. py:attribute:: tick : 最小跳动量
    .. py:attribute:: tickValue : 每一个tick价格
    .. py:attribute:: unit : 每最小变动量价格，即单位价格 = tickValue/tick
    .. py:attribute:: precision : 价格精度
    .. py:attribute:: minTradeNumber : 每笔最小交易量
    .. py:attribute:: maxTradeNumber : 每笔最大交易量
"""


#------------------------------------------------------------------
# from _StockWeight.cpp
#------------------------------------------------------------------

StockWeight.__doc__ = """
    权息记录
    
    .. py:attribute:: datetime : 权息日期
    .. py:attribute:: countAsGift : 每10股送X股
    .. py:attribute:: countForSell : 每10股配X股
    .. py:attribute:: priceForSell : 配股价
    .. py:attribute:: bonus : 每10股红利
    .. py:attribute:: increasement : 每10股转增X股
    .. py:attribute:: totalCount : 总股本（万股）
    .. py:attribute:: freeCount : 流通股（万股）
"""

StockWeightList.__doc__ = """std::vector<StockWeight> 包装"""


#------------------------------------------------------------------
# from _StockManager.cpp
#------------------------------------------------------------------
