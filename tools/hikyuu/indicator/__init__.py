#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20100220, Added by fasiondog
#===============================================================================

#from _indicator import *
from . import _indicator as clib
from hikyuu.util.unicode import (IS_PY3, unicodeFunc, reprFunc)

def indicator_getitem(indicator, i):
    length = len(indicator)
    index = length + i if i < 0 else i
    if index >= length or index < 0:
        raise IndexError("out of range")
    return indicator.get(index, 0)

def indicator_iter(indicator):
    for i in range(len(indicator)):
        yield indicator[i]
        
Indicator = clib.Indicator   
IndicatorImp = clib.IndicatorImp     

if not IS_PY3:
    Indicator.__unicode__ =  unicodeFunc
    Indicator.__repr__ =  reprFunc 

Indicator.__getitem__ = indicator_getitem
Indicator.__iter__ = indicator_iter

Indicator.__add__ = clib.indicator_add
Indicator.__sub__ = clib.indicator_sub
Indicator.__mul__ = clib.indicator_mul
Indicator.__div__ = clib.indicator_div
Indicator.__truediv__ = clib.indicator_div #Python3 div

KDATA = clib.KDATA
OPEN = clib.OPEN
HIGH = clib.HIGH
LOW = clib.LOW
CLOSE = clib.CLOSE
TRANSAMOUNT = clib.TRANSAMOUNT
TRANSCOUNT = clib.TRANSCOUNT
AMO = clib.AMO
VOL = clib.VOL
KDATA_PART = clib.KDATA_PART

PRICELIST = clib.PRICELIST

def MA(data = None, n = 22):
    """ 
    简单移动平均线
    n: 计算均值的周期窗口，必须为大于0的整数
    """
    return clib.MA(data, n) if data else clib.MA(n)

def EMA(data = None, n = 22):
    """
    指数移动平均线(Exponential Moving Average)
    n: 计算均值的周期窗口，必须为大于0的整数
    """
    return clib.EMA(data, n) if data else clib.EMA(n)

def AMA(data = None, n = 10, fast_n = 2, slow_n = 30):
    """
    佩里.J.考夫曼（Perry J.Kaufman）自适应移动平均，参见《精明交易者》（2006年 广东经济出版社）
    n:      计算均值的周期窗口，必须为大于2的整数，默认为10天
    slow_n: 对应慢速EMA线的N值，考夫曼一般设为30，不过当超过60左右该指标会收敛不会有太大的影响
    """
    return clib.AMA(data, n, fast_n, slow_n) if data else clib.AMA(n, fast_n, slow_n)

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

def VIGOR(kdata, n = 2):
    """
    亚历山大.艾尔德力度指数
    参见《走进我的交易室》（2007年 地震出版社） (Alexander Elder) P131
    计算公式：（收盘价今－收盘价昨）＊成交量今
    一般可以再使用EMA进行平滑, 参数n即为EMA平滑参数
    
        VIGOR( (KData)arg1 ) -> Indicator
    """
    return clib.VIGOR(kdata, n)

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

def DIFF(data = None):
    """差分指标，即a[i]-a[i-1]"""
    return clib.DIFF(data) if data else clib.DIFF()

def STDEV(data, n):
    """N日周期内样本标准差"""
    return clib.STDEV(data, n) if data else clib.STDEV(n)