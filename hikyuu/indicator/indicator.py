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

from hikyuu.core import *
from hikyuu import Datetime


def indicator_iter(indicator):
    for i in range(len(indicator)):
        yield indicator[i]


def indicator_getitem(data, i):
    """
    :param i: int | Datetime | slice | str 类型
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
        return data.get_by_datetime(i)

    elif isinstance(i, str):
        return data.get_by_datetime(Datetime(i))

    else:
        raise IndexError("Error index type")


Indicator.__getitem__ = indicator_getitem
Indicator.__iter__ = indicator_iter


VALUE = PRICELIST

try:
    import numpy as np
    import pandas as pd

    def indicator_to_df(indicator):
        """转化为pandas.DataFrame"""
        if indicator.get_result_num() == 1:
            return pd.DataFrame(indicator.to_np(), columns=[indicator.name])

        data = {}
        name = indicator.name
        columns = []
        for i in range(indicator.get_result_num()):
            data[name + str(i)] = indicator.get_result(i)
            columns.append(name + str(i + 1))
        return pd.DataFrame(data, columns=columns)

    Indicator.to_df = indicator_to_df

except:
    print(
        "warning:can't import numpy or pandas lib, ",
        "you can't use method Inidicator.to_np() and to_df!"
    )

VALUE = PRICELIST
