#!/usr/bin/python
# -*- coding: utf8 -*-
# cp936

from hikyuu import Strategy, Query, Datetime, TimeDelta, Seconds, Minutes
from hikyuu import sm


def on_change(stk, spot):
    print(stk.market_code, stk.name, spot.close, spot.bid1, spot.ask1)


def on_spot(rev_time):
    print("rev_time:", rev_time)


def my_func():
    print("calculate:", Datetime.now())
    for s in sm:
        print(s)


# 注意：每一个Strategy 只能作为独立进程执行，即 python xxx.py 的方式执行！
# 以 Strategy 方式运行示例
if __name__ == '__main__':
    s = Strategy(['sh600000', 'sz000001'],  [Query.MIN, Query.DAY])
    # s.run_daily_at(my_func, Datetime.now() - Datetime.today() + Seconds(5))
    s.on_change(on_change)
    s.on_received_spot(on_spot)
    s.run_daily(my_func, Minutes(1))
    s.start()
