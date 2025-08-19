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
import pandas as pd


Indicator.to_numpy = Indicator.to_np
Indicator.to_pandas = Indicator.to_df

Indicator.value_to_numpy = Indicator.value_to_np
Indicator.value_to_pandas = Indicator.value_to_df


def concat_to_df(dates, ind_list, head_stock_code=True, head_ind_name=False):
    """将列表中的指标至合并在一张 pandas DataFrame 中

    :param DatetimeList dates: 指定的日期列表
    :param sequence ind_list: 已计算的指标列表
    :param bool head_ind_name: 表标题是否使用指标名称
    :param bool head_stock_code: 表标题是否使用证券代码
    :return: 合并后的 DataFrame, 以 dates 为 index（注: dates列 为 Datetime 类型）

    示例:
        query = Query(-200)
        k_list = [stk.get_kdata(query) for stk in [sm['sz000001'], sm['sz000002']]]
        ma_list = [MA(CLOSE(k)) for k in k_list]
        concat_to_df(sm.get_trading_calendar(query), ma_list, head_stock_code=True, head_ind_name=False)

    输出：
                date	SZ000001	SZ000002
        0	2023-05-12 00:00:00	12.620000	15.060000
        1	2023-05-15 00:00:00	12.725000	15.060000
        2	2023-05-16 00:00:00	12.690000	15.010000
        3	2023-05-17 00:00:00	12.640000	14.952500
        4	2023-05-18 00:00:00	12.610000	14.886000
        ...	...	...	...
        195	2024-03-01 00:00:00	9.950455	9.837273
        196	2024-03-04 00:00:00	9.995909	9.838182
        197	2024-03-05 00:00:00	10.038182	9.816364
        198	2024-03-06 00:00:00	10.070455	9.776818
        199	2024-03-07 00:00:00	10.101364	9.738182
    """
    df = dates.to_df('ms')
    if not ind_list:
        return df
    for i in range(len(ind_list)):
        ind = ind_list[i]
        if head_ind_name and head_stock_code:
            name = f"{ind.name}/{ind.get_context().get_stock().market_code}"
        elif head_ind_name:
            name = f'{ind.name}{i}'
        else:
            name = ind.get_context().get_stock().market_code
        df = pd.merge(df, ind.to_df()[['datetime', 'value1']].rename(
            columns={'value1': name}), on='datetime', how='left')
    return df


def df_to_ind(df, col_name, col_date=None):
    """
    将 pandas.DataFrame 指定列转化为 Indicator

    示例，从 akshare 获取美国国债10年期收益率:

        import akshare as ak
        df = ak.bond_zh_us_rate("19901219")
        x = df_to_ind(df, '美国国债收益率10年', '日期')

    :param df: pandas.DataFrame
    :param col_name: 指定列名
    :param col_date: 指定日期列名 (为None时忽略, 否则该列为对应参考日期)
    :return: Indicator
    """
    if col_date is not None:
        dates = df[col_date]
        dates = DatetimeList([Datetime(x) for x in dates])
        return PRICELIST(df[col_name], align_dates=dates)
    return PRICELIST(df[col_name])


# 避免 python 中公式原型必须加括号
KDATA = C_KDATA()
CLOSE = C_CLOSE()
OPEN = C_OPEN()
HIGH = C_HIGH()
LOW = C_LOW()
AMO = C_AMO()
VOL = C_VOL()

# 同名指标
VALUE = PRICELIST
CAPITAL = LIUTONGPAN
CONST = LASTVALUE
STD = STDEV
