#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20100220, Added by fasiondog
#===============================================================================

#from ._indicator import *
from . import _indicator as clib
from hikyuu.util.unicode import (IS_PY3, unicodeFunc, reprFunc)
from hikyuu import constant, toPriceList, PriceList

def indicator_getitem(indicator, i):
    if isinstance(i, int):
        length = len(indicator)
        index = length + i if i < 0 else i
        if index >= length or index < 0:
            raise IndexError("out of range")
        return indicator.get(index, 0)
    
    elif isinstance(i, slice):
        return [indicator.get(x, 0) for x in range(*i.indices(len(indicator)))]
    
    else:
        raise IndexError("Error index type")

def indicator_iter(indicator):
    for i in range(len(indicator)):
        yield indicator[i]
        

        
Indicator = clib.Indicator   
IndicatorImp = clib.IndicatorImp     

Indicator.__unicode__ =  unicodeFunc
Indicator.__repr__ =  reprFunc 

Indicator.__getitem__ = indicator_getitem
Indicator.__iter__ = indicator_iter


Indicator.__add__ = clib.indicator_add
Indicator.__sub__ = clib.indicator_sub
Indicator.__mul__ = clib.indicator_mul
Indicator.__div__ = clib.indicator_div
Indicator.__truediv__ = clib.indicator_div #Python3 div

try:
    import numpy as np
    import pandas as pd
    
    def indicator_to_np(indicator):
        """转化为np.array，如果indicator存在多个值，只返回第一个"""
        return np.array(indicator, dtype='d')
    
    def indicator_to_df(indicator):
        if indicator.getResultNumber() == 1:
            return pd.DataFrame(indicator_to_np(indicator), columns=[indicator.name])
    
        data = {}
        name = indicator.name
        columns = []
        for i in range(indicator.getResultNumber()):
            data[name + str(i)] = indicator.getResult(i)
            columns.append(name + str(i+1))
        return pd.DataFrame(data, columns=columns)
    
    Indicator.to_np = indicator_to_np
    Indicator.to_df = indicator_to_df

except:
    pass

OP = clib.Operand

KDATA = clib.KDATA
OPEN = clib.OPEN
HIGH = clib.HIGH
LOW = clib.LOW
CLOSE = clib.CLOSE
AMO = clib.AMO
KDATA_PART = clib.KDATA_PART

POS = clib.POS
#PRICELIST = clib.PRICELIST
VOL = clib.VOL


def MA(data = None, n = 22, type = "SMA"):
    """ 
    移动平均线
    n: 计算均值的周期窗口，必须为大于0的整数
    type: 'SMA' | 'EMA' | 'AMA'
    """
    return clib.MA(data, n, type) if data else clib.MA(n, type)

def SMA(data = None, n = 22):
    """ 
    简单移动平均线
    n: 计算均值的周期窗口，必须为大于0的整数
    """
    return clib.SMA(data, n) if data else clib.SMA(n)


def EMA(data = None, n = 22):
    """
    指数移动平均线(Exponential Moving Average)
    n: 计算均值的周期窗口，必须为大于0的整数
    """
    return clib.EMA(data, n) if data else clib.EMA(n)

def AMA(data = None, n = 10, fast_n = 2, slow_n = 30):
    """
    佩里.J.考夫曼（Perry J.Kaufman）自适应移动平均，参见《精明交易者》（2006年 广东经济出版社）
    n: 计算均值的周期窗口，必须为大于2的整数，默认为10天
    slow_n: 对应慢速EMA线的N值，考夫曼一般设为30，不过当超过60左右该指标会收敛不会有太大的影响
    """
    return clib.AMA(data, n, fast_n, slow_n) if data else clib.AMA(n, fast_n, slow_n)

def DIFF(data = None):
    """差分指标，即a[i]-a[i-1]"""
    return clib.DIFF(data) if data else clib.DIFF()

def MACD(data = None, n1 = 12, n2 = 26, n3 = 9):
    """
    MACD平滑异同移动平均线
    参数： 
        n1: 短期EMA时间窗
        n2: 长期EMA时间窗
        n3: （短期EMA-长期EMA）EMA平滑时间窗
    返回：(MADC_BAR, DIFF, DEA)
    """
    macd = clib.MACD(data, n1, n2, n3) if data else clib.MACD(n1, n2, n3)
    return (macd.getResult(0), macd.getResult(1), macd.getResult(2))


def PRICELIST(data, result_num = 0, discard = 0):
    """
    将data转化为Indicator
    参数：
    data：待转化的数据
    result_num：当data为Indicator实例时，指示Indicator的第几个结果集
    discard：抛弃的数量，在 data 为 Indicator类型时无效
    """
    if isinstance(data, Indicator):
        return clib.PRICELIST(data, result_num)
    elif isinstance(data, PriceList):
        return clib.PRICELIST(data, discard)
    else:
        return clib.PRICELIST(toPriceList(data), discard)

def REF(data = None, n = 1):
    """
    REF 向前引用 （即右移）
    引用若干周期前的数据。
    用法：　REF(X，A)　引用A周期前的X值。
    例如：　REF(CLOSE，1)　表示上一周期的收盘价，在日线上就是昨收。
    n: 引用n周期前的值，即右移n位
    """    
    return clib.REF(data, n) if data else clib.REF(n)

def SAFTYLOSS(data = None, n1 = 10, n2 = 3, p = 2.0):
    """
    亚历山大.艾尔德安全地带止损
    参见《走进我的交易室》（2007年 地震出版社） 亚历山大.艾尔德(Alexander Elder) P202
    计算说明：在回溯周期内（一般为10到20天），将所有向下穿越的长度相加除以向下穿越的次数，
           得到噪音均值（即回溯期内所有最低价低于前一日最低价的长度除以次数），并用今日
           最低价减去（前日噪音均值乘以一个倍数）得到该止损线。为了抵消波动并且保证止损线的
           上移，在上述结果的基础上再取起N日（一般为3天）内的最高值
    注意：返回结果中前（回溯周期宽度＋去最高值的宽度）个点是无效的
    参数：
        n1 整型,计算平均噪音的回溯时间窗口
        n2 整型,对初步止损线去n2日内的最高值
        p 噪音系数
    """
    return clib.SAFTYLOSS(data, n1, n2, p) if data else clib.SAFTYLOSS(n1, n2, p)


def STDEV(data, n):
    """N日周期内样本标准差"""
    return clib.STDEV(data, n) if data else clib.STDEV(n)

def VIGOR(kdata, n = 2):
    """
    亚历山大.艾尔德力度指数
    参见《走进我的交易室》（2007年 地震出版社） (Alexander Elder) P131
    计算公式：（收盘价今－收盘价昨）＊成交量今
    一般可以再使用EMA进行平滑, 参数n即为EMA平滑参数
    
        VIGOR( (KData)arg1 ) -> Indicator
    """
    return clib.VIGOR(kdata, n)
    
def HHV(data, n = 20):
    """
    N日内最高价
    参数：
        n：时间周期窗口
    """
    return clib.HHV(data, n) if data else clib.HHV(data, n)

def LLV(data, n = 20):
    """
    N日内最低价
    参数：
        n：时间周期窗口
    """
    return clib.LLV(data, n) if data else clib.LLV(data, n)