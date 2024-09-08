#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2024-09-08
#    Author: fasiondog

from matplotlib import pylab as plt
from itertools import product
import os
import sys
if sys.platform == 'win32':
    os.system('chcp 65001')

stk_code = "sz000001"
os.environ['HKU_STOCK_LIST'] = stk_code  # 仅加载指定的证券
os.environ['HKU_KTYPE_LIST'] = 'day'  # 加载K线类型（同时包含加载顺序）
# os.environ['HKU_LOAD_STOCK_WEIGHT'] = '0'  # 禁止加载权息数据
# os.environ['HKU_LOAD_HISTORY_FINANCE'] = '0'  # 禁止加载历史财务信息
os.environ['HKU_START_SPOT'] = '0'  # 禁止启动行情接收代理

from hikyuu.interactive import *  # NOQA: E402


fast_n = range(3, 200)
slow_n = range(5, 300)
params = [v for v in product(fast_n, slow_n)]
# print(params)

# sys_list = [get_part('default.sys.趋势双均线', fast_n=x, slow_n=y) for x, y in params]
sys_list = []
for x, y in params:
    if y > x:
        my_sys = get_part('default.sys.趋势双均线', fast_n=x, slow_n=y, tm=crtTM(), mm=MM_Nothing())
        my_sys.name = f'{my_sys.name}_{x}_{y}'
        sys_list.append(my_sys)
print(len(sys_list))


stk = sm[stk_code]
query = Query(-2000)

dates = sm.get_trading_calendar(query)
if len(dates) == 0:
    hku_error(f"Invalid query: {query}")
    exit(-1)

lastdate = dates[-1]


@spend_time
def find_optimal_param(sys_list, stk, query, key=None):
    if key is None:
        key = '帐户平均年收益率%'
    if not Performance.exist(key):
        raise Exception(f"Invalid key: {key}")

    max_val = -10000
    max_sys = None
    for my_sys in sys_list:
        my_sys.run(stk, query)
        per = Performance()
        per.statistics(my_sys.tm, lastdate)
        val = per[key]
        # print(val)
        if val > max_val:
            max_val = val
            max_sys = my_sys
    return max_val, max_sys


while not sm.data_ready:
    import time
    time.sleep(100)

# max_val, max_sys = find_optimal_param(sys_list, stk, query, key='当前总资产')
max_val, max_sys = find_optimal_system_multi(sys_list, stk, query, '当前总资产', 0)

# print(max_val)
print(max_sys.name)
print(len(sys_list))

# max_sys.run(stk, query)
# max_sys.performance()

# plt.show()

# x = find_optimal_system(sys_list, stk, query, '当前总资产', 0)
# print(x)
