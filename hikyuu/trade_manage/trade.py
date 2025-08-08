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

# ===============================================================================
# History:
# 1. 20130213, Added by fasiondog
# ===============================================================================

from hikyuu.util.slice import list_getitem
from hikyuu import *
import numpy as np
import pandas as pd


def TradeList_to_np(data):
    """转化为numpy结构数组"""
    return trades_to_np(data)


def TradeList_to_df(data):
    """转化为pandas的DataFrame"""
    return pd.DataFrame.from_records(trades_to_np(data))


TradeRecordList.to_np = TradeList_to_np
TradeRecordList.to_df = TradeList_to_df


def PositionList_to_np(pos_list):
    """转化为numpy结构数组"""
    return positions_to_np(pos_list)


def PositionList_to_df(pos_list):
    """转化为pandas的DataFrame"""
    return pd.DataFrame.from_records(positions_to_np(pos_list))


PositionRecordList.to_np = PositionList_to_np
PositionRecordList.to_df = PositionList_to_df


def Performance_to_df(per):
    """将 Performance 统计结果转换为 DataFrame 格式"""
    return pd.DataFrame(dict(name=per.names(), value=per.values()))


Performance.to_df = Performance_to_df
