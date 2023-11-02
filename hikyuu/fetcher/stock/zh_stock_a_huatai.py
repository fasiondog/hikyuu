#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2022-08-22
#    Author: fasiondog

import datetime
from hikyuu.util import *


@hku_catch(trace=True, callback=lambda stk: hku_warn("Failed parse stk: {}", stk))
def parse_one_result_huatai(stk):
    result = {}
    if 'OpenPx' not in stk:
        return result

    bids = [x / 10000.0 for x in stk['BuyPriceQueue']]
    bids_amount = [x / 100.0 for x in stk['BuyOrderQtyQueue']]
    asks = [x / 10000.0 for x in stk['SellPriceQueue']]
    asks_amount = [x / 100.0 for x in stk['SellOrderQtyQueue']]

    result['market'] = stk['HTSCSecurityID'][7:]
    result['code'] = stk['HTSCSecurityID'][:6]
    result['name'] = ''
    result['open'] = stk['OpenPx'] / 10000.0  # 今日开盘价

    result['yesterday_close'] = stk['PreClosePx'] / 10000.0  # 昨日收盘价
    result['close'] = stk['ClosePx'] / 10000.0  # 当前价格
    result['high'] = stk['HighPx'] / 10000.0  # 今日最高价
    result['low'] = stk['LowPx'] / 10000.0  # 今日最低价
    result['bid'] = bids[0]  # 竞买价，即“买一”报价
    result['ask'] = asks[0]  # 竞卖价，即“卖一”报价
    result['volume'] = stk['TotalVolumeTrade'] / 100.0  # 成交的股票数，由于股票交易以一百股为基本单位，所以在使用时，通常把该值除以一百
    result['amount'] = stk['TotalValueTrade']  # 成交金额，单位为“元”

    # 买入委托
    for i in range(len(bids)):
        result[f'bid{i}'] = bids[i]
    for i in range(len(bids_amount)):
        result[f'bid{i}_amount'] = bids_amount[i]

    # 卖出委托
    for i in range(len(asks)):
        result[f'ask{i}'] = asks[i]
    for i in range(len(asks_amount)):
        result[f'ask{i}_amount'] = asks_amount[i]

    d = str(stk['MDDate'])
    t = str(stk['MDTime'])
    result['datetime'] = datetime.datetime(int(d[:4]), int(d[4:6]), int(d[6:8]), int(t[:2]), int(t[2:4]), int(t[4:6]))
    return result