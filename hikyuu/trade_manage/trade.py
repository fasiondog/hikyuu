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

from hikyuu import *
import numpy as np
import pandas as pd


TradeRecordList.to_np = lambda self: trades_to_np(self)
TradeRecordList.to_df = lambda self: trades_to_df(self)
TradeRecordList.to_numpy = TradeRecordList.to_np
TradeRecordList.to_pandas = TradeRecordList.to_df
TradeRecordList.to_pyarrow = lambda self: trades_to_pa(self)


PositionRecordList.to_np = lambda self: positions_to_np(self)
PositionRecordList.to_df = lambda self: positions_to_df(self)
PositionRecordList.to_numpy = PositionRecordList.to_np
PositionRecordList.to_pandas = PositionRecordList.to_df
PositionRecordList.to_pyarrow = lambda self: positions_to_pa(self)


def Performance_to_df(per):
    """将 Performance 统计结果转换为 DataFrame 格式"""
    return pd.DataFrame(dict(name=per.names(), value=per.values()))


Performance.to_df = Performance_to_df
