#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#===============================================================================
# 作者：fasiondog
# 历史：1）20100220, Added by fasiondog
#===============================================================================

__version__ = '0.1.0'
__copyright__ = """\
Copyright (c)  Fasiondog; mailto:fasiondog@gmail.com
    See the documentation for further information on copyrights,
    or contact the author. All Rights Reserved.
"""

from ._hikyuu import *
from hikyuu.util.mylog import escapetime

from hikyuu.util.unicode import (unicodeFunc, reprFunc)

MarketInfo.__unicode__ = unicodeFunc
MarketInfo.__repr__ = reprFunc

StockTypeInfo.__unicode = unicodeFunc
StockTypeInfo.__repr__ = reprFunc

KQuery.__unicode__ = unicodeFunc
KQuery.__repr__ = reprFunc

KQueryByDate.__unicode__ = unicodeFunc
KQueryByDate.__repr__ = reprFunc

KRecord.__unicode__ = unicodeFunc
KRecord.__repr__ = reprFunc

KData.__unicode__ = unicodeFunc
KData.__repr__ = reprFunc

Stock.__unicode__ = unicodeFunc
Stock.__repr__ = reprFunc

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
    
    def __init__(self, start = 0, end = constant.null_int64, 
                 kType = KQuery.KType.DAY, recoverType = KQuery.RecoverType.NO_RECOVER):
        super(Query, self).__init__(start, end, kType, recoverType)

QueryByIndex = Query        

class QueryByDate(KQueryByDate):
    """重新定义KQueryByDate，目的如下：
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
    
    def __init__(self, start, end = constant.null_datetime, 
                 kType = Query.DAY, recoverType = Query.NO_RECOVER):
        super(Query, self).__init__(start, end, kType, recoverType)
                

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
    length = len(kdata)
    index = length + i if i < 0 else i
    if index < 0 or index >= length:
        raise IndexError("index out of range: %d" % i)
    return kdata.getKRecord(index)

def KData_iter(kdata):
    for i in range(len(kdata)):
        yield kdata[i]
        
    
KData.__getitem__ = KData_getitem
KData.__iter__ = KData_iter

