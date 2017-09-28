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

from .trade import *
from hikyuu.trade_manage._trade_manage import CostRecord, TradeCostBase,\
    PositionRecord


#------------------------------------------------------------------
# CostRecord
#------------------------------------------------------------------

CostRecord.__doc__ = """交易成本记录"""
CostRecord.commission.__doc__ = """佣金(float)"""
CostRecord.stamptax.__doc__ = """印花税(float)"""
CostRecord.transferfee.__doc__ = """过户费(float)"""
CostRecord.others.__doc__ = """其它费用(float)"""
CostRecord.total.__doc__ = """总成本(float)，= 佣金 + 印花税 + 过户费 + 其它费用"""


#------------------------------------------------------------------
# PositionRecord
#------------------------------------------------------------------

PositionRecord.__doc__ = """持仓记录"""
PositionRecord.stock.__doc__ = """交易对象（Stock）"""
PositionRecord.takeDatetime.__doc__ = """初次建仓时刻（Datetime）"""
PositionRecord.cleanDatetime.__doc__ = """平仓日期，当前持仓记录中为 constant.null_datetime"""
PositionRecord.number.__doc__ = """当前持仓数量（int）"""
