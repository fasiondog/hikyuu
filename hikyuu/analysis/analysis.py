#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2023-10-20
#    Author: fasiondog

import pandas as pd
from hikyuu.core import *


def combinate_ind_analysis(
    stk,
    query,
    tm,
    sys,
    buy_inds,
    sell_inds,
    n=7,
    keys=[
        "Total Invested Principal", "Current Total Assets", "Cash Balance",
        "Open Position Net Value", "Win Rate %", "Number of Winning Trades",
        "Number of Losing Trades"
    ]
):
    '''
    Run the indicator combination test on a single stock

    :param Stock stk: the specified stock
    :param Query query: the specified query condition
    :param TradeManager tm: the trade manager instance
    :param System sys: the system trading strategy instance
    :param list buy_inds: the list of the buy signal generators
    :param list sell_ins: the list of the sell signal generators
    :param int n: the period range existing at the same time when combining the buy signals
    :param list keys: the Performance statistics items to output
    :rtype: pd.DataFrame
    '''
    if not keys:
        for key in keys:
            if not Performance.exist(key):
                raise Exception(f'Invalid key: {key}')

    pers = inner_combinate_ind_analysis(stk, query, tm, sys, buy_inds, sell_inds, n)

    if not keys:
        per = Performance()
        stat_names = per.names()
    else:
        stat_names = keys

    stat_dict = {"证券代码": [], "证券名称": []}
    for name in stat_names:
        stat_dict[name] = []
    index_name = []
    for key in pers:
        index_name.append(key)
        stat_dict["证券代码"].append(stk.code)
        stat_dict["证券名称"].append(stk.name)
        per = pers[key]
        for name in stat_names:
            stat_dict[name].append(per[name])
    return pd.DataFrame(stat_dict, index=index_name)


def combinate_ind_analysis_multi(
    stks,
    query,
    tm,
    sys,
    buy_inds,
    sell_inds,
    n=7,
    keys=[
        "Total Invested Principal", "Current Total Assets", "Cash Balance",
        "Open Position Net Value", "Win Rate %", "Number of Winning Trades",
        "Number of Losing Trades"
    ]
):
    '''
    Run the indicator combination test on the specified stock set

    :param list|Block|sm stks: the specified stock set
    :param Query q: the query condition
    :param TradeManager tm: the trade manager instance
    :param System sys: the system instance
    :param list buy_inds: the list of the buy signal generators
    :param list sell_inds: the list of the sell signal generators
    :param int n: the period when combining the buy signals
    :param list keys: the Performance statistics items to output
    '''
    if not keys:
        for key in keys:
            if not Performance.exist(key):
                raise Exception(f'Invalid key: {key}')

    if isinstance(stks, Block):
        blks = stks
    else:
        blks = Block()
        for stk in stks:
            blks.add(stk)

    out = inner_combinate_ind_analysis_with_block(blks, query, tm, sys, buy_inds, sell_inds, n)
    if not keys:
        ret = out
    else:
        ret = {}
        names = ["组合名称", "证券代码", "证券名称"]
        names.extend(keys)
        for name in names:
            ret[name] = out[name]
    return pd.DataFrame(ret)


def analysis_sys_list(stks, query, sys_proto, keys=[
        "Total Invested Principal", "Current Total Assets", "Cash Balance",
        "Open Position Net Value", "Win Rate %", "Number of Winning Trades",
        "Number of Losing Trades"
    ]):
    if not keys:
        for key in keys:
            if not Performance.exist(key):
                raise Exception(f'Invalid key: {key}')

    names = ["证券代码", "证券名称"]
    names.extend(keys)
    ret = {}
    for name in names:
        ret[name] = []

    per = Performance()
    sys_proto.force_reset_all()
    sys_proto.set_param("shared_ev", False)
    for stk in stks:
        # print(stk)
        k = stk.get_kdata(query)
        my_sys = sys_proto.clone()
        my_sys.run(k, reset_all=True)
        if len(k) > 0:
            per.statistics(my_sys.tm, k[-1].datetime)
            ret["证券代码"].append(stk.market_code)
            ret["证券名称"].append(stk.name)
            for key in keys:
                ret[key].append(per[key])
    return pd.DataFrame(ret)


def analysis_sys_list_multi(stks, query, sys_proto, keys=[
        "Total Invested Principal", "Current Total Assets", "Cash Balance",
        "Open Position Net Value", "Win Rate %", "Number of Winning Trades",
        "Number of Losing Trades"
    ]):
    if not keys:
        for key in keys:
            if not Performance.exist(key):
                raise Exception(f'Invalid key: {key}')

    out = inner_analysis_sys_list(stks, query, sys_proto)
    if not keys:
        ret = out
    else:
        ret = {}
        names = ["证券代码", "证券名称"]
        names.extend(keys)
        for name in names:
            ret[name] = out[name]
    return pd.DataFrame(ret)
