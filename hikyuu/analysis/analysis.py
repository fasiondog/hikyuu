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
    keys=["累计投入本金", "当前总资产", "现金余额", "未平仓头寸净值", "赢利交易比例%", "赢利交易数", "亏损交易数"]
):
    '''
    对单只股票进行指标组合测试

    :param Stock stk: 指定股票
    :param Query query: 指定的查询条件
    :param TradeManager tm: 交易管理实例
    :param System sys: 系统交易策略实例
    :param list buy_inds: 买入信号指标列表
    :param list sell_ins: 卖出信号指标列表
    :param int n: 买入信号组合时同时存在的周期范围
    :param list keys: 输出 Performance 统计项
    :rtype: pd.DataFrame
    '''
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
    keys=["累计投入本金", "当前总资产", "现金余额", "未平仓头寸净值", "赢利交易比例%", "赢利交易数", "亏损交易数"]
):
    '''
    对指定的股票集进行指标组合测试

    :param list|Block|sm stks: 指定的股票集
    :param Query q: 查询条件
    :param TradeManager tm: 交易管理实例
    :param System sys: 系统实例
    :param list buy_inds: 买入信号指标列表
    :param list sell_inds: 卖出信号指标列表
    :param int n: 买入信号组合时的周期
    :param list keys: 输出 Performance 统计项
    '''
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


def analysis_sys_list(stks, query, sys_proto, keys=["累计投入本金", "当前总资产", "现金余额", "未平仓头寸净值", "赢利交易比例%", "赢利交易数", "亏损交易数"]):
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


def analysis_sys_list_multi(stks, query, sys_proto, keys=["累计投入本金", "当前总资产", "现金余额", "未平仓头寸净值", "赢利交易比例%", "赢利交易数", "亏损交易数"]):
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
