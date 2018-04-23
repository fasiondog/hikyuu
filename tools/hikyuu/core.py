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


import sys
if sys.version > '3':
    IS_PY3 = True
else:
    IS_PY3 = False

from .core_doc import *
from hikyuu.util.mylog import escapetime
from hikyuu.util.slice import list_getitem
from hikyuu.util.unicode import (unicodeFunc, reprFunc)
from datetime import date, datetime


#------------------------------------------------------------------
# 常量定义，各种C++中Null值
#------------------------------------------------------------------

constant = Constant()


#------------------------------------------------------------------
# 支持Python的__unicode__、__repr
#------------------------------------------------------------------

MarketInfo.__unicode__ = unicodeFunc
MarketInfo.__repr__ = reprFunc

StockTypeInfo.__unicode = unicodeFunc
StockTypeInfo.__repr__ = reprFunc

KQuery.__unicode__ = unicodeFunc
KQuery.__repr__ = reprFunc

KRecord.__unicode__ = unicodeFunc
KRecord.__repr__ = reprFunc

KData.__unicode__ = unicodeFunc
KData.__repr__ = reprFunc

Stock.__unicode__ = unicodeFunc
Stock.__repr__ = reprFunc

Block.__unicode__ = unicodeFunc
Block.__repr__ = reprFunc

Datetime.__unicode__ = unicodeFunc
Datetime.__repr__ = reprFunc

Parameter.__unicode__ = unicodeFunc
Parameter.__repr__ = reprFunc


#------------------------------------------------------------------
# 增加Datetime、Stock的hash支持，以便可做为dict的key
#------------------------------------------------------------------

def datetime_hash(self):
    return self.number

def stock_hash(self):
    return self.id

Datetime.__hash__ = datetime_hash
Stock.__hash__ = stock_hash


#------------------------------------------------------------------
# 增强 Datetime
#------------------------------------------------------------------

__old_Datetime_init__ = Datetime.__init__

def __new_Datetime_init__(self, var = None):
    """
    日期时间类（精确到秒），通过以下方式构建：
    
    - 通过字符串：Datetime("2010-1-1 10:00:00")
    - 通过 Python 的date：Datetime(date(2010,1,1))
    - 通过 Python 的datetime：Datetime(datetime(2010,1,1,10)
    - 通过 YYYYMMDDHHMM 形式的整数：Datetime(201001011000)
    
    获取日期列表参见： :py:func:`getDateRange`
    
    获取交易日日期参见： :py:meth:`StockManager.getTradingCalendar` 
    """    
    if var is None:
        __old_Datetime_init__(self)
    
    #datetime实例同时也是date的实例，判断必须放在date之前
    elif isinstance(var, datetime):
        __old_Datetime_init__(self, str(var))
    elif isinstance(var, date):
        __old_Datetime_init__(self, "{} 00".format(var))
    
    elif isinstance(var, str):
        if var.find(' ') == -1:
            __old_Datetime_init__(self, "{} 00".format(var))
        else:
            __old_Datetime_init__(self, var)
    else:
        __old_Datetime_init__(self, var)

def Datetime_date(self):
    """转化生成 python 的 date"""
    return date(self.year, self.month, self.day)

def Datetime_datetime(self):
    """转化生成 python 的 datetime"""
    return datetime(self.year, self.month, self.day, 
                    self.hour, self.minute, self.second)
        
def Datetime_isNull(self):
    """是否是Null值, 即是否等于 constant.null_datetime"""
    return True if self == constant.null_datetime else False

Datetime.__init__ =  __new_Datetime_init__
Datetime.date = Datetime_date
Datetime.datetime = Datetime_datetime
Datetime.isNull = Datetime_isNull 


#------------------------------------------------------------------
#重定义KQuery
#------------------------------------------------------------------

class Query(KQuery):
    """重新定义KQuery，目的如下：
    1、使用短类名
    2、使用短枚举类型
    3、利用Python命名参数优点
    """
    INDEX = KQuery.QueryType.INDEX
    DATE = KQuery.QueryType.DATE
    DAY = KQuery.KType.DAY
    WEEK = KQuery.KType.WEEK
    MONTH = KQuery.KType.MONTH
    QUARTER = KQuery.KType.QUARTER
    HALFYEAR = KQuery.KType.HALFYEAR
    YEAR = KQuery.KType.YEAR
    MIN = KQuery.KType.MIN
    MIN3 = KQuery.KType.MIN3
    MIN5 = KQuery.KType.MIN5
    MIN15 = KQuery.KType.MIN15
    MIN30 = KQuery.KType.MIN30
    MIN60 = KQuery.KType.MIN60
    HOUR2 = KQuery.KType.HOUR2
    HOUR4 = KQuery.KType.HOUR4
    HOUR6 = KQuery.KType.HOUR6
    HOUR12 = KQuery.KType.HOUR12
    NO_RECOVER = KQuery.RecoverType.NO_RECOVER
    FORWARD = KQuery.RecoverType.FORWARD
    BACKWARD = KQuery.RecoverType.BACKWARD
    EQUAL_FORWARD = KQuery.RecoverType.EQUAL_FORWARD
    EQUAL_BACKWARD = KQuery.RecoverType.EQUAL_BACKWARD
    
    def __init__(self, start = 0, end = None, 
                 kType = KQuery.KType.DAY, recoverType = KQuery.RecoverType.NO_RECOVER):
        """
        构建按索引 [start, end) 方式获取K线数据条件
        
        :param ind start: 起始日期
        :param ind end: 结束日期
        :param KQuery.KType kType: K线数据类型（如日线、分钟线等）
        :param KQuery.RecoverType recoverType: 复权类型
        :return: 查询条件
        :rtype: KQuery
        """
        end_pos = constant.null_int64 if end is None else end
        super(Query, self).__init__(start, end_pos, kType, recoverType)

QueryByIndex = Query

def QueryByDate(start=None, end=None, kType=Query.DAY, 
                recoverType=Query.NO_RECOVER):
    """
    构建按日期 [start, end) 方式获取K线数据条件
    
    :param Datetime start: 起始日期
    :param Datetime end: 结束日期
    :param KQuery.KType kType: K线数据类型（如日线、分钟线等）
    :param KQuery.RecoverType recoverType: 复权类型
    :return: 查询条件
    :rtype: KQuery        
    """
    start_date = Datetime.min() if start is None else start
    end_date = Datetime.max() if end is None else end
    return KQueryByDate(start_date, end_date, kType, recoverType)


KQuery.INDEX = KQuery.QueryType.INDEX
KQuery.DATE = KQuery.QueryType.DATE
KQuery.DAY = KQuery.KType.DAY
KQuery.WEEK = KQuery.KType.WEEK
KQuery.MONTH = KQuery.KType.MONTH
KQuery.QUARTER = KQuery.KType.QUARTER
KQuery.HALFYEAR = KQuery.KType.HALFYEAR
KQuery.YEAR = KQuery.KType.YEAR
KQuery.MIN = KQuery.KType.MIN
KQuery.MIN5 = KQuery.KType.MIN5
KQuery.MIN15 = KQuery.KType.MIN15
KQuery.MIN30 = KQuery.KType.MIN30
KQuery.MIN60 = KQuery.KType.MIN60
KQuery.NO_RECOVER = KQuery.RecoverType.NO_RECOVER
KQuery.FORWARD = KQuery.RecoverType.FORWARD
KQuery.BACKWARD = KQuery.RecoverType.BACKWARD
KQuery.EQUAL_FORWARD = KQuery.RecoverType.EQUAL_FORWARD
KQuery.EQUAL_BACKWARD = KQuery.RecoverType.EQUAL_BACKWARD


#------------------------------------------------------------------
# 增强 KData 的遍历
#------------------------------------------------------------------

def KData_getitem(kdata, i):
    if isinstance(i, int):
        length = len(kdata)
        index = length + i if i < 0 else i
        if index < 0 or index >= length:
            raise IndexError("index out of range: %d" % i)
        return kdata.getKRecord(index)
    
    elif isinstance(i, Datetime):
        return kdata.getKRecordByDate(i)
    
    elif isinstance(i, slice):
        return [kdata.getKRecord(x) for x in range(*i.indices(len(kdata)))]
        
    else:
        raise IndexError("Error index type")
        return KRecord()

def KData_iter(kdata):
    for i in range(len(kdata)):
        yield kdata[i]
        
def KData_getPos(kdata, datetime):
    """
    获取指定时间对应的索引位置
    
    :param Datetime datetime: 指定的时间
    :return: 对应的索引位置，如果不在数据范围内，则返回 None
    """
    pos = kdata._getPos(datetime)
    return pos if pos != constant.null_size else None


KData.__getitem__ = KData_getitem
KData.__iter__ = KData_iter
KData.getPos = KData_getPos


#------------------------------------------------------------------
# 封装增强其他C++ vector类型的遍历
#------------------------------------------------------------------
        
PriceList.__getitem__ = list_getitem
DatetimeList.__getitem__ = list_getitem
StringList.__getitem__ = list_getitem
BlockList.__getitem__ = list_getitem


#------------------------------------------------------------------
# 增加转化为 np.array、pandas.DataFrame 的功能
#------------------------------------------------------------------

try:
    import numpy as np
    import pandas as pd
    
    def KData_to_np(kdata):
        """转化为numpy结构数组"""
        k_type = np.dtype({'names':['datetime','open', 'high', 'low','close', 
                                    'amount', 'volume'], 
                'formats':['datetime64[D]','d','d','d','d','d','d']})
        return np.array([(k.datetime, k.openPrice, k.highPrice, 
                          k.lowPrice, k.closePrice, k.transAmount, 
                          k.transCount) for k in kdata], dtype=k_type)
        
    def KData_to_df(kdata):
        """转化为pandas的DataFrame"""
        return pd.DataFrame.from_records(KData_to_np(kdata), index='datetime')    

    KData.to_np = KData_to_np
    KData.to_df = KData_to_df
    
    def PriceList_to_np(data):
        """仅在安装了numpy模块时生效，转换为numpy.array"""
        return np.array(data, dtype='d')
       
    def PriceList_to_df(data):
        """仅在安装了pandas模块时生效，转换为pandas.DataFrame"""
        return pd.DataFrame(data.to_np(), columns=('Value',))
        
    PriceList.to_np = PriceList_to_np
    PriceList.to_df = PriceList_to_df
    
    def DatetimeList_to_np(data):
        """仅在安装了numpy模块时生效，转换为numpy.array"""
        return np.array(data, dtype='datetime64[D]')
        
    def DatetimeList_to_df(data):
        """仅在安装了pandas模块时生效，转换为pandas.DataFrame"""
        return pd.DataFrame(data.to_np(), columns=('Datetime',))
        
    DatetimeList.to_np = DatetimeList_to_np
    DatetimeList.to_df = DatetimeList_to_df
    
except:
    pass


#------------------------------------------------------------------
# 净化命名空间
#------------------------------------------------------------------

__all__ = [#类
           'Block', 
           'BlockList', 
           'Datetime', 
           'DatetimeList', 
           'KData',
           'KQuery', 
           'KQueryByDate', 
           'KQueryByIndex', 
           'KRecord', 
           'KRecordList', 
           'MarketInfo', 
           'Parameter', 
           'PriceList',
           'Query', 
           'QueryByDate', 
           'QueryByIndex', 
           'Stock', 
           'StockManager', 
           'StockTypeInfo', 
           'StockWeight', 
           'StockWeightList', 
           'StringList',
           
           #变量
           'constant', 
           'IS_PY3',
           
           #函数
           'getDateRange', 
           'getStock',
           'hikyuu_init', 
           'hku_load', 
           'hku_save', 
           'roundDown',
           'roundUp', 
           'toPriceList', 
           
           #包
           #'util'
           ]
