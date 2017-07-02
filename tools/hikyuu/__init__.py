#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20100220, Added by fasiondog
#===============================================================================

__version__ = '1.0.3'
__copyright__ = """\
Copyright (c)  Fasiondog; mailto:fasiondog@gmail.com
    See the documentation for further information on copyrights,
    or contact the author. All Rights Reserved.
"""

import sys
if sys.version > '3':
    IS_PY3 = True
else:
    IS_PY3 = False

from ._hikyuu import *
from hikyuu.util.mylog import escapetime

from hikyuu.util.unicode import (unicodeFunc, reprFunc)
from datetime import date, datetime

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

#常量定义，各种C++中Null值
constant = Constant()


#定义hash
def datetime_hash(self):
    return self.number

def stock_hash(self):
    return self.id

Datetime.__hash__ = datetime_hash
Stock.__hash__ = stock_hash


#================================================================
# Datetime支持直接从date, datetime变量初始化
__old_Datetime_init__ = Datetime.__init__

def __new_Datetime_init__(self, var = None):
    if var is None:
        __old_Datetime_init__(self)
    elif isinstance(var, date):
        __old_Datetime_init__(self, "{} 00".format(var))
    elif isinstance(var, datetime):
        __old_Datetime_init__(self, str(var))
    elif isinstance(var, str):
        if var.find(' ') == -1:
            __old_Datetime_init__(self, "{} 00".format(var))
        else:
            __old_Datetime_init__(self, var)
    else:
        __old_Datetime_init__(self, var)

def Datetime_date(self):
    return date(self.year, self.month, self.day)
        
Datetime.__init__ =  __new_Datetime_init__
Datetime.date = Datetime_date


#================================================================ 
#重定义KQuery
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
    MIN5 = KQuery.KType.MIN5
    MIN15 = KQuery.KType.MIN15
    MIN30 = KQuery.KType.MIN30
    MIN60 = KQuery.KType.MIN60
    NO_RECOVER = KQuery.RecoverType.NO_RECOVER
    FORWARD = KQuery.RecoverType.FORWARD
    BACKWARD = KQuery.RecoverType.BACKWARD
    EQUAL_FORWARD = KQuery.RecoverType.EQUAL_FORWARD
    EQUAL_BACKWARD = KQuery.RecoverType.EQUAL_BACKWARD
    
    def __init__(self, start = 0, end = None, 
                 kType = KQuery.KType.DAY, recoverType = KQuery.RecoverType.NO_RECOVER):
        """
        构建按索引方式 [start, end) 查询K线数据条件，查询[start, end)的K线记录
        start: int 起始位置（默认0）
        end  : int 结束位置（默认为全部数量）
        kType: KQuery.KType.DAY K线类型（默认为日线）
        recoverType: KQuery.RecoverType.NO_RECOVER 复权类型（默认不复权）
        """
        end_pos = constant.null_int64 if end is None else end
        super(Query, self).__init__(start, end_pos, kType, recoverType)

QueryByIndex = Query

def QueryByDate(start=None, end=None, kType=Query.DAY, 
                recoverType=Query.NO_RECOVER):
    """
    构建按日期 [start, end) 查询K线数据条件
    start: Datetime      起始日期（默认为支持的最小日期）
    end  : Datetime      结束日期（默认为支持的最大日期）
    kType: KQuery.KType  K线类型（默认为日线）
    recoverType: KQuery.RecoverType 复权类型（默认不复权）
    
    return KQuery K线数据查询条件
        
    """
    start_date = Datetime.minDatetime() if start is None else start
    end_date = Datetime.maxDatetime() if end is None else end
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

#================================================================
#封装KData
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
    pos = kdata._getPos(datetime)
    return pos if pos != constant.null_size else None


KData.__getitem__ = KData_getitem
KData.__iter__ = KData_iter
KData.getPos = KData_getPos


#================================================================
def list_getitem(data, i):
    if isinstance(i, int):
        length = len(data)
        index = length + i if i < 0 else i
        if index < 0 or index >= length:
            raise IndexError("index out of range: %d" % i)
        return data.get(index)
    
    elif isinstance(i, slice):
        return [data.get(x) for x in range(*i.indices(len(data)))]
    
    else:
        raise IndexError("Error index type")
        
PriceList.__getitem__ = list_getitem
DatetimeList.__getitem__ = list_getitem
StringList.__getitem__ = list_getitem
BlockList.__getitem__ = list_getitem


#================================================================
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
       return np.array(data, dtype='d')
       
    def PriceList_to_df(data):
        return pd.DataFrame(data.to_np(), columns=('Value',))
        
    PriceList.to_np = PriceList_to_np
    PriceList.to_df = PriceList_to_df
    
    def DatetimeList_to_np(data):
        return np.array(data, dtype='datetime64[D]')
        
    def DatetimeList_to_df(data):
        return pd.DataFrame(data.to_np(), columns=('Datetime',))
        
    DatetimeList.to_np = DatetimeList_to_np
    DatetimeList.to_df = DatetimeList_to_df
    
except:
    pass