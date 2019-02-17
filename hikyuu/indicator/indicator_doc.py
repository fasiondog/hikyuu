#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936
#
# The MIT License (MIT)
#
# Copyright (c) 2017 fasiondog
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

from .indicator import *


AMA.__doc__ = """
AMA([data, n=10, fast_n=2, slow_n=30])

    佩里.J 考夫曼（Perry J.Kaufman）自适应移动平均 [BOOK1]_
    
    :param Indicator data: 输入数据
    :param int n: 计算均值的周期窗口，必须为大于2的整数
    :param int fast_n: 对应快速周期N
    :param int slow_n: 对应慢速EMA线的N值
    :return: Indicator
    
    * result(0): AMA
    * result(1): ER
"""


AMO.__doc__ = """
AMO([data])

    获取成交金额，包装KData的成交金额成Indicator
    
    :param data: 输入数据（KData 或 Indicator）
    :return: Indicator
"""


CLOSE.__doc__ = """
CLOSE([data])
   
    获取收盘价，包装KData的收盘价成Indicator
    
    :param data: 输入数据（KData 或 Indicator）
    :return: Indicator
"""


CVAL.__doc__ = """
    CVAL(data[, value=0.0])
    
        data 为 Indicator 实例，创建和 data 等长的常量指标，其值和为value，抛弃长度discard和data一样
    
    CVAL([value=0.0, len=0, discard=0])
    
        按指定的长度、抛弃数量创建常量指标
    
    :param Indicator data: Indicator实例
    :param float value: 常数值
    :param int len: 长度
    :param int discard: 抛弃数量
    :return: Indicator
"""


DIFF.__doc__ = """
DIFF([data])

    差分指标，即data[i] - data[i-1]
    
    :param Indicator data: 输入数据
    :return: Indicator
"""


EMA.__doc__ = """
EMA([data, n=22])

    指数移动平均线(Exponential Moving Average)

    :param Indicator data: 输入数据
    :param int n: 计算均值的周期窗口，必须为大于0的整数
    :return: Indicator     
"""


HHV.__doc__ = """
HHV([data, n=20])

    N日内最高价

    :param Indicator data: 输入数据
    :param int n: N日时间窗口
    :return: Indicator
"""


HIGH.__doc__ = """
HIGH([data])

    获取最高价，包装KData的最高价成Indicator

    :param data: 输入数据（KData 或 Indicator） 
    :return: Indicator
"""


KDATA.__doc__ = """
KDATA([data])

    包装KData成Indicator，用于其他指标计算

    :param data: KData 或 具有6个返回结果的Indicator（如KDATA生成的Indicator）
    :return: Indicator
"""


KDATA_PART.__doc__ = """
KDATA_PART([data, kpart])

    根据字符串选择返回指标KDATA/OPEN/HIGH/LOW/CLOSE/AMO/VOL，如:KDATA_PART("CLOSE")等同于CLOSE()

    :param data: 输入数据（KData 或 Indicator） 
    :param string kpart: KDATA|OPEN|HIGH|LOW|CLOSE|AMO|VOL
    :return: Indicator
"""


LLV.__doc__ = """
LLV([data, n=20])

    N日内最低价

    :param data: 输入数据
    :param int n: N日时间窗口
    :return: Indicator
"""


LOW.__doc__ = """
LOW([data])

    获取最低价，包装KData的最低价成Indicator
    
    :param data: 输入数据（KData 或 Indicator） 
    :return: Indicator
"""


MA.__doc__ = """
MA([data, n=22, type="SMA"])

    移动平均数包装，默认为简单平均数
    
    :param Indicator data: 输入数据
    :param int n: 时间窗口
    :param string type: "EMA"|"SMA"|"AMA"
    :return: Indicator
"""


MACD.__doc__ = """
MACD([data, n1=12, n2=26, n3=9])

    平滑异同移动平均线
    
    :param Indicator data: 输入数据
    :param int n1: 短期EMA时间窗
    :param int n2: 长期EMA时间窗
    :param int n3: （短期EMA-长期EMA）EMA平滑时间窗
    :return: 具有三个结果集的 Indicator

    * result(0): MACD_BAR：MACD直柱，即MACD快线－MACD慢线
    * result(1): DIFF: 快线,即（短期EMA-长期EMA）
    * result(2): DEA: 慢线，即快线的n3周期EMA平滑
"""


OPEN.__doc__ = """
OPEN([data])

    获取开盘价，包装KData的开盘价成Indicator
    
    :param data: 输入数据（KData 或 Indicator） 
    :return: Indicator
"""


REF.__doc__ = """
REF([data, n])

    向前引用 （即右移），引用若干周期前的数据。
    用法：REF(X，A)　引用A周期前的X值。
    
    :param Indicator data: 输入数据
    :param int n: 引用n周期前的值，即右移n位
    :return: Indicator
"""    

SAFTYLOSS.__doc__ = """
SAFTYLOSS([data, n1=10, n2=3, p=2.0])

    亚历山大 艾尔德安全地带止损线，参见 [BOOK2]_
    
    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的上移，在上述结果的基础上再取起N日（一般为3天）内的最高值

    :param Indicator data: 输入数据
    :param int n1: 计算平均噪音的回溯时间窗口
    :param int n2: 对初步止损线去n2日内的最高值
    :param float p: 噪音系数
    :return: Indicator
"""


SMA.__doc__ = """
SMA([data, n=22])

    简单移动平均线
    
    :param Indicator data: 输入数据
    :param int n: 时间窗口
    :return: Indicator
"""


STDEV.__doc__ = """
STDEV([data, n=10])

    计算N周期内样本标准差
    
    :param Indicator data: 输入数据
    :param int n: 时间窗口
    :return: Indicator
"""


VIGOR.__doc__ = """
VIGOR(data[, n=2])

    亚历山大.艾尔德力度指数 [BOOK2]_
    
    计算公式：（收盘价今－收盘价昨）＊成交量今
    
    :param data: 输入数据（KData 或 具有6个结果集的Indicator）
    :param int n: EMA平滑窗口
    :return: Indicator
"""


VOL.__doc__ = """
VOL([data])

    获取成交量，包装KData的成交量成Indicator

    :param data: 输入数据（KData 或 Indicator）
    :return: Indicator
"""
    
    
