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


def TradeList_to_np(t_list):
    """转化为numpy结构数组"""
    t_type = np.dtype(
        {
            'names': [
                '交易日期', '证券代码', '证券名称', '业务名称', '计划交易价格', '实际成交价格', '目标价格', '成交数量', '佣金', '印花税', '过户费', '其它成本',
                '交易总成本', '止损价', '现金余额', '信号来源'
            ],
            'formats':
            ['datetime64[D]', 'U10', 'U20', 'U10', 'd', 'd', 'd', 'i', 'd', 'd', 'd', 'd', 'd', 'd', 'd', 'U5']
        }
    )
    return np.array(
        [
            (
                t.datetime, t.stock.market_code, t.stock.name, get_business_name(t.business), t.plan_price,
                t.real_price, t.goal_price, t.number, t.cost.commission, t.cost.stamptax, t.cost.transferfee,
                t.cost.others, t.cost.total, t.stoploss, t.cash, get_system_part_name(t.part)
            ) for t in t_list
        ],
        dtype=t_type
    )


def TradeList_to_df(t):
    """转化为pandas的DataFrame"""
    return pd.DataFrame.from_records(TradeList_to_np(t), index='交易日期')


TradeRecordList.to_np = TradeList_to_np
TradeRecordList.to_df = TradeList_to_df


def PositionList_to_np(pos_list):
    """转化为numpy结构数组"""
    t_type = np.dtype(
        {
            'names': ['证券代码', '证券名称', '买入日期', '已持仓天数', '持仓数量', '投入金额', '当前市值', '盈亏金额', '盈亏比例'],
            'formats': ['U10', 'U20', 'datetime64[D]', 'i', 'i', 'd', 'd', 'd', 'd']
        }
    )
    sm = StockManager.instance()
    query = Query(-1)
    data = []
    for pos in pos_list:
        invest = pos.buy_money - pos.sell_money + pos.total_cost
        k = pos.stock.get_kdata(query)
        cur_val = k[0].close * pos.number
        bonus = cur_val - invest
        date_list = sm.get_trading_calendar(Query(Datetime(pos.take_datetime.date())))
        data.append(
            (
                pos.stock.market_code, pos.stock.name, pos.take_datetime, len(date_list), pos.number, invest,
                cur_val, bonus, 100 * bonus / invest
            )
        )
    return np.array(data, dtype=t_type)


def PositionList_to_df(pos_list):
    """转化为pandas的DataFrame"""
    return pd.DataFrame.from_records(PositionList_to_np(pos_list), index='证券代码')


PositionRecordList.to_np = PositionList_to_np
PositionRecordList.to_df = PositionList_to_df


def Performance_to_df(per):
    """将 Performance 统计结果转换为 DataFrame 格式"""
    return pd.DataFrame(dict(name=per.names(), value=per.values()))


Performance.to_df = Performance_to_df
