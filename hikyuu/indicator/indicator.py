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

from ._indicator import *
from hikyuu.util.unicode import (IS_PY3, unicodeFunc, reprFunc)
from hikyuu import constant, toPriceList, PriceList, Datetime


def indicator_iter(indicator):
    for i in range(len(indicator)):
        yield indicator[i]


def indicator_getitem(data, i):
    """对C++引出的vector，实现python的切片，
       将引入的vector类的__getitem__函数覆盖即可。
    """
    if isinstance(i, int):
        length = len(data)
        index = length + i if i < 0 else i
        if index < 0 or index >= length:
            raise IndexError("index out of range: %d" % i)
        return data.get(index)
    
    elif isinstance(i, slice):
        return [data.get(x) for x in range(*i.indices(len(data)))]

    elif isinstance(i, Datetime):
        return data.getByDate(i)

    elif isinstance(i, str):
        return data.getByDate(Datetime(i))

    else:
        raise IndexError("Error index type")


Indicator.__getitem__ = indicator_getitem
Indicator.__iter__ = indicator_iter

Indicator.__unicode__ =  unicodeFunc
Indicator.__repr__ =  reprFunc 

Indicator.__add__ = indicator_add
Indicator.__sub__ = indicator_sub
Indicator.__mul__ = indicator_mul
Indicator.__div__ = indicator_div
Indicator.__mod__ = indicator_mod
Indicator.__truediv__ = indicator_div #Python3 div
Indicator.__eq__ = indicator_eq
Indicator.__ne__ = indicator_ne
Indicator.__ge__ = indicator_ge
Indicator.__le__ = indicator_le
Indicator.__gt__ = indicator_gt
Indicator.__lt__ = indicator_lt
Indicator.__and__ = indicator_and
Indicator.__or__ = indicator_or

Indicator.__radd__ = lambda self, other: self.__add__(other)
Indicator.__rsub__ = lambda self, other: CVAL(self, other).__sub__(self)
Indicator.__rmul__ = lambda self, other: self.__mul__(other)
Indicator.__rdiv__ = lambda self, other: CVAL(self, other).__div__(self)
Indicator.__rmod__ = lambda self, other: CVAL(self, other).__mod__(self)
Indicator.__rtruediv__ = lambda self, other: CVAL(self, other).__truediv__(self)
Indicator.__req__ = lambda self, other: self.__eq__(other)
Indicator.__rne__ = lambda self, other: self.__ne__(other)
Indicator.__rge__ = lambda self, other: CVAL(self, other).__ge__(self)
Indicator.__rle__ = lambda self, other: CVAL(self, other).__le__(self)
Indicator.__rgt__ = lambda self, other: CVAL(self, other).__gt__(self)
Indicator.__rlt__ = lambda self, other: CVAL(self, other).__lt__(self)
Indicator.__rand__ = lambda self, other: CVAL(self, other).__and__(self)
Indicator.__ror__ = lambda self, other: CVAL(self, other).__or__(self)


KDATA = IKDATA()
OPEN = IOPEN()
CLOSE = ICLOSE()
HIGH = IHIGH()
LOW = ILOW()
AMO = IAMO()
VOL = IVOL()


def PRICELIST(data, result_num=0, discard=0):
    """
    将 list、tuple、Indicator 转化为普通的 Indicator
    
    :param data: 输入数据，可以为 list、tuple、Indicator
    :param int result_num: 当data为Indicator实例时，指示Indicator的第几个结果集
    :param int discard: 在 data 为 Indicator类型时无效。表示前端抛弃的数据点数，抛弃的值使用 constant.null_price 填充
    :return: Indicator
    """
    from . import _indicator as ind
    if isinstance(data, ind.Indicator):
        return ind.PRICELIST(data, result_num)
    else:
        return ind.PRICELIST(toPriceList(data), discard)
    

try:
    import numpy as np
    import pandas as pd
    
    def indicator_to_np(indicator):
        """转化为np.array，如果indicator存在多个值，只返回第一个"""
        return np.array(indicator, dtype='d')
    
    def indicator_to_df(indicator):
        """转化为pandas.DataFrame"""
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
    print("warning:can't import numpy or pandas lib, ",  
          "you can't use method Inidicator.to_np() and to_df!")

