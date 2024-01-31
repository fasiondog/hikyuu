# coding:utf-8
#
# The MIT License (MIT)
#
# Created on: 2020-11-29
#     Author: fasiondog

import datetime
from pytdx.hq import TdxHq_API
from hikyuu.data.common_pytdx import search_best_tdx

from hikyuu.util import *


@hku_catch(ret=None, callback=lambda quotes: hku_warn(quotes))
def parse_one_result(quotes):
    result = {}
    hku_check_ignore(quotes, "Invalid input param!")

    result['market'] = 'SH' if quotes['market'] == 1 else 'SZ'
    result['code'] = quotes['code']
    result['name'] = ''
    result['open'] = quotes['open']  # 今日开盘价
    result['yesterday_close'] = quotes['last_close']  # 昨日收盘价
    result['close'] = quotes['price']  # 当前价格
    result['high'] = quotes['high']  # 今日最高价
    result['low'] = quotes['low']  # 今日最低价
    result['bid'] = float(quotes['bid1'])  # 竞买价，即“买一”报价
    result['ask'] = float(quotes['ask1'])  # 竞卖价，即“卖一”报价
    result['volume'] = float(quotes['vol'])  # 成交的股票手数
    result['amount'] = round(quotes['amount'] / 1000.0, 2)  # 成交金额，单位为“元”，若要以“万元”为成交金额的单位，需要把该值除以一万
    result['bid1_amount'] = float(quotes['bid_vol1'])  # “买一”申请4695股，即47手
    result['bid1'] = float(quotes['bid1'])  # “买一”报价
    result['bid2_amount'] = float(quotes['bid_vol2'])
    result['bid2'] = float(quotes['bid2'])
    result['bid3_amount'] = float(quotes['bid_vol3'])
    result['bid3'] = float(quotes['bid3'])
    result['bid4_amount'] = float(quotes['bid_vol4'])
    result['bid4'] = float(quotes['bid4'])
    result['bid5_amount'] = float(quotes['bid_vol5'])
    result['bid5'] = float(quotes['bid5'])
    result['ask1_amount'] = float(quotes['ask_vol1'])  # “卖一”申报3100股，即31手
    result['ask1'] = float(quotes['ask1'])  # “卖一”报价
    result['ask2_amount'] = float(quotes['ask_vol2'])
    result['ask2'] = float(quotes['ask2'])
    result['ask3_amount'] = float(quotes['ask_vol3'])
    result['ask3'] = float(quotes['ask3'])
    result['ask4_amount'] = float(quotes['ask_vol4'])
    result['ask4'] = float(quotes['ask4'])
    result['ask5_amount'] = float(quotes['ask_vol5'])
    result['ask5'] = float(quotes['ask5'])
    result['datetime'] = datetime.datetime.combine(
        datetime.date.today(), datetime.time.fromisoformat(quotes['servertime'])
    )
    return result


@hku_catch(ret=[])
def request_data(api, stklist, parse_one_result):
    """请求失败将抛出异常"""
    quotes_list = api.get_security_quotes(stklist)
    result = [parse_one_result(q) for q in quotes_list] if quotes_list is not None else []
    return [r for r in result if r is not None]


def get_spot(stocklist, ip, port, batch_func=None):
    api = TdxHq_API()
    hku_check(api.connect(ip, port), 'Failed connect tdx ({}:{})!'.format(ip, port))

    count = 0
    tmplist = []
    result = []
    max_size = 80
    for stk in stocklist:
        tmplist.append((1 if stk.market == 'SH' else 0, stk.code))
        count += 1
        if count >= max_size:
            phase_result = request_data(api, tmplist, parse_one_result)
            if phase_result:
                result.extend(phase_result)
                if batch_func:
                    batch_func(phase_result)
            count = 0
            tmplist = []
    if tmplist:
        phase_result = request_data(api, tmplist, parse_one_result)
        if phase_result:
            result.extend(phase_result)
            if batch_func:
                batch_func(phase_result)
    return result
