#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

#
# 警告：Hikyuu 为量化研究工具，本身不包含程序化交易接口。此部分仅为策略调度运行时示例，
#      供自行实现程序化交易时参考，请自行负责程序化交易可能造成的损失。
#

from hikyuu import *


def on_change(stg: Strategy, stk: Stock, spot: SpotRecord):
    print("[on_change]:", stk.market_code, stk.name, spot.close, spot.bid1, spot.ask1)


def on_spot(stg: Strategy, rev_time: Datetime):
    print("[on_received_spot] rev_time:", rev_time)


def my_func1(stg: Strategy):
    print("[my_func1]", str(stg.now()))


def my_func2(stg: Strategy):
    print("[my_func2] calculate:", stg.now())
    for s in sm:
        print(s)


# 注意：
#   1.每一个Strategy 只能作为独立进程执行，即 python xxx.py 的方式执行！
#   2.请开启 HikyuuTdx 行情采集，否则接收不到数据
# Strategy 方式运行示例
if __name__ == '__main__':
    # 创建策略运行时，必须指定 stock 和 ktype 列表
    # strategy 只会加载指定的 stock, ktype 的数据，行情接收也只会更新这些数据
    # 如需使用交易日历，请记得同时指定 sh000001
    s = Strategy(['sh600000', 'sz000001'],  [Query.MIN, Query.DAY])

    # 当前自动延迟10秒/20秒后执行，忽略节假日限制
    s.run_daily_at(my_func1, Datetime.now() - Datetime.today() + Seconds(10), False)
    s.run_daily_at(my_func1, Datetime.now() - Datetime.today() + Seconds(20), False)

    # 收到指定 stock 的行情更新
    s.on_change(on_change)

    # 收到行情更新
    s.on_received_spot(on_spot)

    # 每隔 1 分钟循环一次 (ignore_market 忽略开闭市时间限制, 否则仅在开盘期间执行)
    s.run_daily(my_func2, Minutes(1))  # , ignore_market=True)
    s.start()
